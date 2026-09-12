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

## Threads and shutdown

Pattern and level/color updates may be made from application threads while the
scheduler advances the animation. The default scheduler has its own dispatch
thread; a supplied scheduler must have one dispatch thread. Animation state and
stepper scheduling are synchronized separately. LED implementations must keep
any other access to their hardware state synchronized as appropriate.

Both `Blinker` and `RgbBlinker` provide `shutdown()`. It permanently stops the
stepper and waits for claimed callbacks before animation state can be torn down.
Later pattern and level/color updates are ignored. Shutdown preserves the last
LED output; call `turnOff()` first if that is the desired final output.

```cpp
blinker.turnOff();
blinker.shutdown();  // Keep the LED and scheduler alive until this returns.
```

Destructors perform shutdown before member teardown. Prevent concurrent public
calls before destroying the blinker. Do not hold a lock required by a callback
or LED implementation while waiting. From the blinker's own callback, shutdown
returns false instead of waiting on itself and still disables stepper scheduling;
call it again from outside the callback to finish shutdown. Do not destroy a
blinker from its callback. The LED and scheduler must outlive the blinker.

These lifecycle operations require the corresponding `roo_scheduler` shutdown
API when building the two libraries from local checkouts.

## Host emulation

Host builds use the roo_testing 2.0 Arduino ESP32 profile. With Bazelisk 1.21
or newer, a plain command defaults to that profile and prints a notice:

    bazel test ...
    bazel test ... --config=asan
    bazel test ... --config=roo_testing_arduino_esp32

The files under .roo_testing/bazelrc/esp32 are vendored from roo_testing;
follow their canonical-source headers when refreshing them.

The basic monochrome example is also a directly runnable emulator target:

    bazel run //examples/monochrome/Trivial:Trivial

For an emulator-visible version of the smooth fade pattern, run:

    bazel run //examples/monochrome/VoltageTrace:VoltageTrace

`VoltageTrace` attaches a voltage sink to the built-in LED pin and prints CSV
rows whenever LEDC assigns a PWM signal and every 100 ms thereafter. The rows
contain emulated uptime plus DC, total RMS, and AC RMS voltage, making the
pulsating LED observable in a terminal. Stop it with Ctrl-C.

When developing it alongside an unreleased local `roo_testing` checkout, use
that checkout's LEDC implementation explicitly:

    bazel run //examples/monochrome/VoltageTrace:VoltageTrace \
      --override_repository=roo_testing+=$PWD/../roo_testing
