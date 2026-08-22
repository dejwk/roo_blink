#pragma once

#include <cstdint>

// Minimal host-emulation stand-in for the Adafruit_NeoPixel API used by these
// examples. Hardware builds should continue to use the real library.
class Adafruit_NeoPixel {
 public:
  Adafruit_NeoPixel(uint16_t, int) {}

  void setPixelColor(uint16_t, uint8_t, uint8_t, uint8_t) {}
  void show() {}
};
