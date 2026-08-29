#include "roo_blink/monochrome/led_esp32.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "roo_logging.h"

#if defined(ESP32)

namespace roo_blink {
namespace esp32 {

static constexpr ledc_timer_bit_t kDutyRes = LEDC_TIMER_10_BIT;
static constexpr int kDuty = 1 << kDutyRes;

static constexpr int kFreq = 40000;

// Function-local construction makes first-use initialization once-only without
// running LEDC code during static GpioLed/BuiltinLed construction.
class FadeService {
 public:
  FadeService() {
    CHECK_EQ(xTaskGetSchedulerState(), taskSCHEDULER_RUNNING)
        << "LEDC fades require a running FreeRTOS task";
    const esp_err_t result = ledc_fade_func_install(0);
    // ESP-IDF reports INVALID_STATE when another LEDC client already installed
    // the process-wide service.
    CHECK(result == ESP_OK || result == ESP_ERR_INVALID_STATE)
        << "ledc_fade_func_install failed: " << result;
  }
};

void EnsureFadeServiceInstalled() {
  static const FadeService service;
}

GpioLed::GpioLed(int gpio_num, Mode mode, ledc_timer_t timer_num,
                 ledc_channel_t channel)
    : channel_(channel), mode_(mode) {
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                    .duty_resolution = kDutyRes,
                                    .timer_num = timer_num,
                                    .freq_hz = kFreq,
                                    .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {.gpio_num = gpio_num,
                                        .speed_mode = LEDC_LOW_SPEED_MODE,
                                        .channel = channel,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .timer_sel = timer_num,
                                        .duty = static_cast<uint32_t>(dutyForLevel(0)),
                                        .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

}

void GpioLed::setLevel(uint16_t level) {
  ledc_set_duty(LEDC_LOW_SPEED_MODE, channel_, dutyForLevel(level));
  ledc_update_duty(LEDC_LOW_SPEED_MODE, channel_);
}

bool GpioLed::fade(uint16_t target_level, roo_time::Duration duration) {
  EnsureFadeServiceInstalled();
  if (ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, channel_,
                              dutyForLevel(target_level),
                              duration.inMillis()) != ESP_OK) {
    return false;
  }
  return ledc_fade_start(LEDC_LOW_SPEED_MODE, channel_, LEDC_FADE_NO_WAIT) ==
         ESP_OK;
}

int GpioLed::dutyForLevel(uint16_t level) const {
  const uint32_t brightness_count =
      (static_cast<uint64_t>(level) * kDuty + 32767) / 65535;
  return mode_ == ON_HIGH ? brightness_count : kDuty - brightness_count;
}

#if CONFIG_IDF_TARGET_ESP32C3
static const int kBuiltinLedPin = 8;
static const GpioLed::Mode kBuiltinLedMode = GpioLed::ON_HIGH;
#else
static const int kBuiltinLedPin = 2;
static const GpioLed::Mode kBuiltinLedMode = GpioLed::ON_HIGH;
#endif

Led& BuiltinLed() {
  static GpioLed instance(kBuiltinLedPin, kBuiltinLedMode);
  return instance;
}

}  // namespace esp32
}  // namespace roo_blink

#endif
