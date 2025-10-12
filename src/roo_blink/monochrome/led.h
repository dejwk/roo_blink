#pragma once

#include "roo_time.h"
#include "stdint.h"

namespace roo_blink {

// Abstract interface representing a monochrome LED.
class Led {
 public:
  // Sets the LED to the specified brightness, in the range from 0 (turned off)
  // to 65535 (maximum brightness).
  virtual void setLevel(uint16_t level) = 0;

  // Sets the LED to the maximum brightness.
  void turnOn() { setLevel(65535); }

  // Turns the LED completely off.
  void turnOff() { setLevel(0); }

  // Initiates a linear fade from the current brightness level to the specified
  // brightness level, expected to take the specified time interval to complete.
  virtual bool fade(uint16_t target_level, roo_time::Duration duration) = 0;
};

}  // namespace roo_blink
