#include <Arduino.h>

#include "roo_blink.h"
#include "roo_time.h"

using namespace roo_blink;

Blinker blinker(roo_blink::esp32::BuiltinLed());

void setup() {
  Serial.begin(115200);
  BlinkSequence sequence;
  sequence.add(TurnOn());
  sequence.add(Hold(roo_time::Millis(100)));
  sequence.add(TurnOff());
  sequence.add(Hold(roo_time::Millis(100)));
  sequence.add(FadeOn(roo_time::Millis(200)));
  sequence.add(Hold(roo_time::Millis(100)));
  sequence.add(FadeOff(roo_time::Millis(200)));
  sequence.add(Hold(roo_time::Millis(100)));
  sequence.add(FadeTo(8000, roo_time::Millis(100)));
  sequence.add(FadeTo(0, roo_time::Millis(100)));
  sequence.add(FadeTo(16000, roo_time::Millis(100)));
  sequence.add(FadeTo(0, roo_time::Millis(100)));
  sequence.add(FadeTo(24000, roo_time::Millis(100)));
  sequence.add(FadeTo(0, roo_time::Millis(100)));
  sequence.add(FadeTo(32000, roo_time::Millis(100)));
  sequence.add(FadeTo(0, roo_time::Millis(100)));
  sequence.add(FadeTo(48000, roo_time::Millis(100)));
  sequence.add(FadeTo(0, roo_time::Millis(100)));
  sequence.add(FadeTo(65535, roo_time::Millis(100)));
  blinker.loop(std::move(sequence));
}

void loop() {
  // You're free to do as you please; it will not interfefe with the blinker.
}
