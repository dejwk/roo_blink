#include <Arduino.h>

#include "roo_blink.h"
#include "roo_blink/rgb/adafruit_neopixel_led.h"
#include "roo_time.h"

using namespace roo_blink;

static const int kLedPin = 8;

Adafruit_NeoPixel pixels(1, kLedPin);
NeoPixelLed led(pixels);

RgbBlinker blinker(led);

void setup() {
  // Starts blinking purple, with 50% intensity (expressed in the RGB color
  // value), 30% duty cycle (i.e. 30% rising, 30% falling), with fast (30%)
  // rampup of the 'on' state, and slow (90%) rampdown of the off state.
  blinker.loop(
      RgbBlink(roo_time::Millis(2000), Color(127, 0, 127), 30, 30, 90));
}

void loop() {
  // You're free to do as you please; it will not interfefe with the blinker.
}
