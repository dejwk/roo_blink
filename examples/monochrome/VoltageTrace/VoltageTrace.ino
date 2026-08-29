#include <Arduino.h>

#include <cstdio>

#include "roo_blink.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/system/timer.h"
#include "roo_testing/transducers/voltage/voltage.h"
#include "roo_testing/transducers/voltage/voltage_signal.h"
#include "roo_time.h"

namespace {

constexpr int kLedPin = 2;

roo_testing_transducers::SimpleVoltageSink sink =
    roo_testing_transducers::SimpleVoltageSink::WithSignalCallback(
        "builtin-led",
        [](const roo_testing_transducers::VoltageSignal& signal) {
          const int64_t now = system_time_get_micros();
          const roo_testing_transducers::VoltageAnalysis analysis =
              roo_testing_transducers::AnalyzeVoltage(signal, now);
          if (signal.kind() ==
              roo_testing_transducers::VoltageSignalKind::kSquare) {
            const auto& square =
                std::get<roo_testing_transducers::SquareVoltageSpec>(
                    signal.spec());
            if (const auto* fade = std::get_if<
                    roo_testing_transducers::LinearDutyFade>(&square.duty)) {
              std::printf(
                  "fade-start,%lld,start-duty=%.1f%%,target-duty=%.1f%%,"
                  "duration=%llums,dc=%.3fV,rms=%.3fV,ac-rms=%.3fV\n",
                  static_cast<long long>(now), fade->start_duty * 100,
                  fade->target_duty * 100,
                  static_cast<unsigned long long>(fade->duration_us / 1000),
                  analysis.dc_voltage, analysis.rms_voltage,
                  analysis.ac_rms_voltage);
              std::fflush(stdout);
              return;
            }
          }
          std::printf(
              "signal-set,%lld,dc=%.3fV,rms=%.3fV,ac-rms=%.3fV\n",
              static_cast<long long>(now), analysis.dc_voltage,
              analysis.rms_voltage, analysis.ac_rms_voltage);
          std::fflush(stdout);
        });

roo_blink::Blinker* blinker = nullptr;

}  // namespace

void setup() {
  // Attach before configuring LEDC, so each assigned PWM envelope is visible.
  FakeEsp32().gpio.attachOutput(kLedPin, sink);
  static roo_blink::esp32::GpioLed led_instance(
      kLedPin, roo_blink::esp32::GpioLed::ON_HIGH);
  static roo_blink::Blinker blinker_instance(led_instance);
  blinker = &blinker_instance;
  blinker->loop(roo_blink::Blink(roo_time::Millis(1000), 30, 30, 90));
}

void loop() {
  const int64_t now = system_time_get_micros();
  const roo_testing_transducers::VoltageAnalysis analysis =
      roo_testing_transducers::AnalyzeVoltage(*sink.signal(), now);
  std::printf("sample,%lld,dc=%.3fV,rms=%.3fV,ac-rms=%.3fV\n",
              static_cast<long long>(now), analysis.dc_voltage,
              analysis.rms_voltage, analysis.ac_rms_voltage);
  std::fflush(stdout);
  delay(100);
}
