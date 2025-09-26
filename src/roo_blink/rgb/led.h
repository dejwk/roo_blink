#pragma once

#include "stdint.h"
#include "roo_time.h"

#include "roo_blink/rgb/color.h"

namespace roo_blink {

class RgbLed {
 public:
  virtual void setColor(Color color);
};

}