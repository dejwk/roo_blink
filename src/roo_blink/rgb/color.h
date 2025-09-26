#pragma once

#include <inttypes.h>

namespace roo_blink {

class Color {
 public:
  constexpr Color() : rgb_(0) {}
  
  constexpr Color(uint8_t r, uint8_t g, uint8_t b)
      : rgb_((r << 16) | (g << 8) | b) {}

  uint8_t r() const { return (uint8_t)(rgb_ >> 16); }
  uint8_t g() const { return (uint8_t)(rgb_ >> 8); }
  uint8_t b() const { return (uint8_t)(rgb_ >> 0); }

 private:
  uint32_t rgb_;
};

}  // namespace roo_led