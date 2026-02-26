#pragma once

#include <inttypes.h>

namespace roo_blink {

/// Simple 24-bit RGB color value.
class Color {
 public:
  /// Creates black (0,0,0).
  constexpr Color() : rgb_(0) {}

  /// Creates a color from 8-bit RGB components.
  constexpr Color(uint8_t r, uint8_t g, uint8_t b)
      : rgb_((r << 16) | (g << 8) | b) {}

  /// Returns the red component.
  uint8_t r() const { return (uint8_t)(rgb_ >> 16); }
  /// Returns the green component.
  uint8_t g() const { return (uint8_t)(rgb_ >> 8); }
  /// Returns the blue component.
  uint8_t b() const { return (uint8_t)(rgb_ >> 0); }

 private:
  uint32_t rgb_;
};

}  // namespace roo_blink