They say that Arduino is only good for blinking LEDs.

Well, that might not be _entirely_ true. But if you _do_ blink LEDs, do it like a pro!

This library makes it easy to implement LED signaling. It works with monochrome LEDs as well as RGB LEDs. You can use simple blinking patterns, or customize them. And it is not just on/off. Monochrome LEDs can be faded in or out. For RGB LEDs, you can define patterns that smoothly transition through colors.

Importantly, all this is handled asynchronously (fire and forget), so that you can focus on your business logic and need not worry about updating LED state.

The following is a simple complete example that produces a nicely-looking blinking pattern for the built-in LED (on ESP32):

```cpp
#include <Arduino.h>

#include "roo_blink.h"
#include "roo_time.h"

using namespace roo_blink;

Blinker blinker(roo_blink::esp32::BuiltinLed());

void setup() {
  // Starts a customized blink sequence: 30% duty cycle (i.e. 30% rising, 30%
  // falling), with fast (30%) rampup of the 'on' state, and slow (90%) rampdown
  // of the off state. This config results in the following timings:
  // * 90ms (30% * 30%) ramp up from 0 to 100% brightness;
  // * 210ms (30% * 70%) hold at 100% brightness;
  // * 630ms (70% * 90%) ramp down from 100% to 0% brightness;
  // * 70ms (70% * 10%) hold at 0% brightness.
  blinker.loop(Blink(roo_time::Millis(1000), 30, 30, 90));
}

void loop() {
  // You're free to do as you please; it will not interfefe with the blinker.
}
```
