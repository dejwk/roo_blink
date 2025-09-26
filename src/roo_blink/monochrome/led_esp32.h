#pragma once

#include "roo_blink/monochrome/led.h"

#if defined(ESP32)

#include "driver/ledc.h"

namespace roo_blink {
namespace esp32 {

// Monochrome LED connected to a GPIO pin, using the ESP32 LEDC PWM
// functionality to control brightness.
class GpioLed : public ::roo_blink::Led {
 public:
  enum Mode { ON_HIGH, ON_LOW };

  // Constructs a GpioLed connected to the specified GPIO pin.
  // The pin is expected to be connected to the LED anode if mode is ON_HIGH
  // (the default), or to the LED cathode if mode is ON_LOW.
  //
  // If provided, the specified LEDC timer and channel will be used to
  // control the LED brightness. The default timer and channel are suitable
  // for most applications, unless multiple LEDs are being controlled.
  GpioLed(int gpio_num, Mode mode = ON_LOW,
          ledc_timer_t timer_num = LEDC_TIMER_0,
          ledc_channel_t channel = LEDC_CHANNEL_0);

  void setLevel(uint16_t level) override;
  bool fade(uint16_t target_level, roo_time::Interval duration) override;

 private:
  int dutyForLevel(uint16_t level) const;

  ledc_channel_t channel_;
  Mode mode_;
};

// Returns a GpioLed representing the built-in LED on the ESP32 board.
Led& BuiltinLed();

}  // namespace esp32
}  // namespace roo_blink

#endif
