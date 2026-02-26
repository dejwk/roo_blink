#pragma once

#include <Arduino.h>

#include <vector>

#include "roo_blink/rgb/led.h"
#include "roo_logging.h"
#include "roo_scheduler.h"
#include "roo_time.h"

namespace roo_blink {

/// Single step of an RGB blink sequence.
class RgbStep {
 public:
  friend constexpr RgbStep RgbSetTo(Color color);
  friend constexpr RgbStep RgbHold(roo_time::Duration duration);
  friend constexpr RgbStep RgbTurnOff();
  friend constexpr RgbStep RgbFadeTo(Color color, roo_time::Duration duration);
  friend constexpr RgbStep RgbFadeOff(roo_time::Duration duration);

 private:
  friend class RgbBlinker;

  enum Type { kSet, kHold, kFade };

  constexpr RgbStep(Type type, Color color, uint16_t duration_millis);

  Type type_;
  Color target_color_;
  uint16_t duration_millis_;
};

/// Sequence of steps for RGB blinking.
class RgbBlinkSequence {
 public:
  void add(RgbStep step) { sequence_.push_back(std::move(step)); }

 private:
  std::vector<RgbStep> sequence_;

  friend class RgbBlinker;
};

/// Creates a step that sets the LED to the specified color instantly.
constexpr RgbStep RgbSetTo(Color color);

/// Creates a step that disables the LED. Equivalent to RgbSetTo(Color()).
constexpr RgbStep RgbTurnOff();

/// Creates a step that fades to the target color over the duration.
constexpr RgbStep RgbFadeTo(Color color, roo_time::Duration duration);

/// Creates a step that fades the LED off over the duration.
constexpr RgbStep RgbFadeOff(roo_time::Duration duration);

/// Creates a step that holds the current color for the duration.
constexpr RgbStep RgbHold(roo_time::Duration duration);

/// Runs blink sequences on an RGB LED.
class RgbBlinker {
 public:
  /// Constructs a RgbBlinker using the default scheduler.
  RgbBlinker(RgbLed& led);

  /// Constructs a RgbBlinker using the specified scheduler.
  RgbBlinker(RgbLed& led, roo_scheduler::Scheduler& scheduler);

  /// Repeats the sequence indefinitely.
  void loop(RgbBlinkSequence sequence);

  /// Repeats the sequence the specified number of times.
  void repeat(RgbBlinkSequence sequence, int repetitions,
              Color terminal_color = Color());

  /// Executes the sequence once.
  void execute(RgbBlinkSequence sequence, Color terminal_color = Color());

  /// Enables the LED, setting it to the specified color.
  void setColor(Color color);

  /// Disables the LED.
  void turnOff();

 private:
  void updateSequence(RgbBlinkSequence sequence, int repetitions,
                      Color terminal_color);
  void step();

  RgbLed& led_;
  roo_scheduler::SingletonTask stepper_;
  std::vector<RgbStep> sequence_;
  Color current_color_;
  Color terminal_color_;
  size_t repetitions_;
  size_t pos_;

  mutable roo::mutex mutex_;

  bool fade_in_progress_;
  Color fade_start_color_;
  Color fade_target_color_;
  roo_time::Uptime fade_start_time_;
  roo_time::Uptime fade_end_time_;
};

/// Creates a symmetric blink sequence with optional ramp-up/down segments.
RgbBlinkSequence RgbBlink(roo_time::Duration period, Color color,
                          int duty_percent = 50, int rampup_percent_on = 0,
                          int rampup_percent_off = 0);

// Implementation details.

constexpr RgbStep::RgbStep(Type type, Color color, uint16_t duration_millis)
    : type_(type), target_color_(color), duration_millis_(duration_millis) {}

constexpr RgbStep RgbSetTo(Color color) {
  return RgbStep(RgbStep::kSet, color, 0);
}

constexpr RgbStep RgbTurnOff() { return RgbSetTo(Color()); }

constexpr RgbStep RgbHold(roo_time::Duration duration) {
  return RgbStep(RgbStep::kHold, Color(), (uint16_t)duration.inMillis());
}

constexpr RgbStep RgbFadeTo(Color color, roo_time::Duration duration) {
  return RgbStep(RgbStep::kFade, color, (uint16_t)duration.inMillis());
}

constexpr RgbStep RgbFadeOff(roo_time::Duration duration) {
  return RgbFadeTo(Color(), duration);
}

}  // namespace roo_blink