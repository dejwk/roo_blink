#pragma once

#include "roo_time.h"
#include "stdint.h"

namespace roo_blink {

/// Abstract interface representing a monochrome LED.
class Led {
 public:
  /// Sets the LED brightness in the range 0 (off) to 65535 (max).
  virtual void setLevel(uint16_t level) = 0;

  /// Sets the LED to the maximum brightness.
  void turnOn() { setLevel(65535); }

  /// Turns the LED completely off.
  void turnOff() { setLevel(0); }

  /// Initiates a linear fade to the target level over the duration.
  virtual bool fade(uint16_t target_level, roo_time::Duration duration) = 0;
};

}  // namespace roo_blink
