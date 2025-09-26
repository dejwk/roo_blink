#include <Arduino.h>

#include "roo_blink.h"
#include "roo_time.h"

using namespace roo_blink;

Blinker blinker(roo_blink::esp32::BuiltinLed());

void setup() {
  Serial.begin(115200);
  // Starts a trivial blink sequence: 500ms on, 500ms off, indefinitely.
  blinker.loop(Blink(roo_time::Millis(500)));
}

void loop() {
  // You're free to do as you please; it will not interfefe with the blinker.
}
