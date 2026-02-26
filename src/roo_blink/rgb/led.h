#pragma once

#include "roo_blink/rgb/color.h"
#include "roo_time.h"
#include "stdint.h"

namespace roo_blink {

/// Abstract interface representing an RGB LED.
class RgbLed {
 public:
  /// Sets the LED to the specified color.
  virtual void setColor(Color color);
};

}  // namespace roo_blink