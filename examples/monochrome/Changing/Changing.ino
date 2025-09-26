#include <Arduino.h>

#include "roo_blink.h"
#include "roo_scheduler.h"
#include "roo_time.h"

using namespace roo_blink;

Blinker blinker(roo_blink::esp32::BuiltinLed());

void setBlinkPattern(int pattern) {
  switch (pattern) {
    case 0: {
      blinker.loop(Blink(roo_time::Millis(1000), 30, 30, 90));
      break;
    }
    case 1: {
      blinker.loop(Blink(roo_time::Millis(500), 70, 50, 50));
      break;
    }
    default: {
      blinker.loop(Blink(roo_time::Millis(300), 30, 20, 20));
      break;
    }
  }
}

std::atomic<int> current_blink_pattern{0};
roo_scheduler::Scheduler scheduler;

roo_scheduler::RepetitiveTask change_blink_pattern_task(
    scheduler, roo_time::Seconds(5), []() {
      int next_pattern = (current_blink_pattern + 1) % 3;
      Serial.printf("Switching to blink pattern %d\n", next_pattern);
      setBlinkPattern(next_pattern);
      current_blink_pattern = next_pattern;
    });

void setup() {
  Serial.begin(115200);
  change_blink_pattern_task.startInstantly();
}

void loop() { scheduler.run(); }
