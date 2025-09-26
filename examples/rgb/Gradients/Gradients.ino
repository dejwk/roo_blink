#include <Arduino.h>

#include "roo_blink.h"
#include "roo_blink/rgb/adafruit_neopixel_led.h"
#include "roo_time.h"

using namespace roo_blink;

static const int kLedPin = 8;

Adafruit_NeoPixel pixels(1, kLedPin);
NeoPixelLed led(pixels);

RgbBlinker blinker(led);

using namespace roo_time;

void setup() {
  // Create a custom loop that goes through several colors, stopping at some briefly.
  RgbBlinkSequence sequence;
  sequence.add(RgbFadeTo(Color(255, 0, 0), Millis(300)));
  sequence.add(RgbHold(Millis(50)));
  sequence.add(RgbFadeTo(Color(0, 255, 0), Millis(300)));
  sequence.add(RgbHold(Millis(50)));
  sequence.add(RgbFadeTo(Color(0, 0, 255), Millis(300)));
  sequence.add(RgbHold(Millis(50)));
  blinker.loop(std::move(sequence));
}

void loop() {
  // You're free to do as you please; it will not interfefe with the blinker.
}
