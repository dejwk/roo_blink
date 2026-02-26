#pragma once

#include "Adafruit_NeoPixel.h"
#include "roo_blink/rgb/led.h"

namespace roo_blink {

/// RGB LED backed by an Adafruit_NeoPixel instance.
class NeoPixelLed : public RgbLed {
 public:
  /// Creates a wrapper for a specific NeoPixel index.
  NeoPixelLed(Adafruit_NeoPixel& neopixel, int led_idx = 0)
      : neopixel_(neopixel), led_idx_(led_idx) {}

  /// Sets the color and immediately updates the strip.
  void setColor(Color color) override {
    neopixel_.setPixelColor(led_idx_, color.r(), color.g(), color.b());
    neopixel_.show();
  }

 private:
  Adafruit_NeoPixel& neopixel_;
  int led_idx_;
};

}  // namespace roo_blink