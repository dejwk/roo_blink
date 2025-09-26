#include "roo_blink/rgb/blinker.h"

#include "roo_blink.h"
#include "roo_blink/default_scheduler.h"
#include "roo_logging.h"

using namespace roo_time;

namespace roo_blink {

RgbBlinker::RgbBlinker(RgbLed& led) : RgbBlinker(led, DefaultScheduler()) {}

RgbBlinker::RgbBlinker(RgbLed& led, roo_scheduler::Scheduler& scheduler)
    : led_(led),
      stepper_(scheduler, [this]() { step(); }),
      sequence_(),
      pos_(0) {}

void RgbBlinker::loop(RgbBlinkSequence sequence) {
  updateSequence(std::move(sequence), -1, Color());
}

void RgbBlinker::repeat(RgbBlinkSequence sequence, int repetitions,
                        Color terminal_color) {
  updateSequence(std::move(sequence), repetitions - 1, terminal_color);
}

void RgbBlinker::execute(RgbBlinkSequence sequence, Color terminal_color) {
  updateSequence(std::move(sequence), 0, terminal_color);
}

void RgbBlinker::setColor(Color color) { updateSequence({}, 0, color); }

void RgbBlinker::turnOff() { updateSequence({}, 0, Color()); }

void RgbBlinker::updateSequence(RgbBlinkSequence sequence, int repetitions,
                                Color terminal_color) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  sequence_ = std::move(sequence.sequence_);
  terminal_color_ = terminal_color;
  current_color_ = terminal_color;
  repetitions_ = repetitions;
  fade_in_progress_ = false;
  pos_ = 0;
  if (!sequence_.empty() && !stepper_.is_scheduled()) {
    stepper_.scheduleNow(roo_scheduler::PRIORITY_ELEVATED);
  } else {
    current_color_ = terminal_color;
    led_.setColor(current_color_);
  }
}

void RgbBlinker::step() {
  roo::lock_guard<roo::mutex> lock(mutex_);
  if (fade_in_progress_) {
    roo_time::Uptime now = roo_time::Uptime::Now();
    if (now >= fade_end_time_) {
      fade_in_progress_ = false;
      current_color_ = fade_target_color_;
      led_.setColor(current_color_);
    } else {
      float progress = (now - fade_start_time_).inMillisFloat() /
                       (fade_end_time_ - fade_start_time_).inMillisFloat();
      uint8_t new_r = (uint8_t)((float)fade_start_color_.r() +
                                ((float)fade_target_color_.r() -
                                 (float)fade_start_color_.r()) *
                                    progress);
      uint8_t new_g = (uint8_t)((float)fade_start_color_.g() +
                                ((float)fade_target_color_.g() -
                                 (float)fade_start_color_.g()) *
                                    progress);
      uint8_t new_b = (uint8_t)((float)fade_start_color_.b() +
                                ((float)fade_target_color_.b() -
                                 (float)fade_start_color_.b()) *
                                    progress);
      current_color_ = Color(new_r, new_g, new_b);
      led_.setColor(current_color_);
      stepper_.scheduleAfter(roo_time::Millis(20),
                             roo_scheduler::PRIORITY_ELEVATED);
      return;
    }
  }
  uint16_t next_delay = 0;
  do {
    if (pos_ >= sequence_.size()) {
      sequence_.clear();
      return;
    }
    const RgbStep& s = sequence_[pos_];
    switch (s.type_) {
      case RgbStep::kSet: {
        current_color_ = s.target_color_;
        led_.setColor(current_color_);
        break;
      }
      case RgbStep::kHold: {
        next_delay = s.duration_millis_;
        break;
      }
      case RgbStep::kFade:
      default: {
        fade_in_progress_ = true;
        fade_start_color_ = current_color_;
        fade_target_color_ = s.target_color_;
        fade_start_time_ = roo_time::Uptime::Now();
        fade_end_time_ =
            fade_start_time_ + roo_time::Millis(s.duration_millis_);
        next_delay = 20;
        break;
      }
    }
    ++pos_;
    if (pos_ == sequence_.size() && repetitions_ != 0) {
      if (repetitions_ > 0) --repetitions_;
      pos_ = 0;
    }
  } while (next_delay == 0);
  stepper_.scheduleAfter(roo_time::Millis(next_delay),
                         roo_scheduler::PRIORITY_ELEVATED);
}

RgbBlinkSequence RgbBlink(roo_time::Interval period, Color color,
                          int duty_percent, int rampup_percent_on,
                          int rampup_percent_off) {
  CHECK_GE(duty_percent, 0);
  CHECK_LE(duty_percent, 100);
  CHECK_GE(rampup_percent_on, 0);
  CHECK_LE(rampup_percent_on, 100);
  CHECK_GE(rampup_percent_off, 0);
  CHECK_LE(rampup_percent_off, 100);
  int millis = period.inMillis();
  int millis_1st = duty_percent * millis / 100;
  int millis_1st_rampup = rampup_percent_on * millis_1st / 100;
  int millis_2nd = millis - millis_1st;
  int millis_2nd_rampup = rampup_percent_off * millis_2nd / 100;

  RgbBlinkSequence result;

  if (millis_1st_rampup > 0) {
    result.add(RgbFadeTo(color, Millis(millis_1st_rampup)));
  } else {
    result.add(RgbSetTo(color));
  }
  if (millis_1st_rampup < millis_1st) {
    result.add(RgbHold(Millis(millis_1st - millis_1st_rampup)));
  }

  if (millis_2nd_rampup > 0) {
    result.add(RgbFadeOff(Millis(millis_2nd_rampup)));
  } else {
    result.add(RgbTurnOff());
  }
  if (millis_2nd_rampup < millis_2nd) {
    result.add(RgbHold(Millis(millis_2nd - millis_2nd_rampup)));
  }

  return result;
}

}  // namespace roo_blink
