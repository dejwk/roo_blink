#include "roo_blink/monochrome/blinker.h"

#include "roo_blink.h"
#include "roo_blink/default_scheduler.h"
#include "roo_logging.h"
#include "roo_threads.h"

using namespace roo_time;

namespace roo_blink {

Blinker::Blinker(Led& led) : Blinker(led, DefaultScheduler()) {}

Blinker::Blinker(Led& led, roo_scheduler::Scheduler& scheduler)
    : led_(led),
      stepper_(scheduler, [this]() { step(); }),
      sequence_(),
      pos_(0) {}

void Blinker::loop(BlinkSequence sequence) {
  updateSequence(std::move(sequence), -1, 0);
}

void Blinker::repeat(BlinkSequence sequence, int repetitions,
                     uint16_t terminal_level) {
  updateSequence(std::move(sequence), repetitions - 1, terminal_level);
}

void Blinker::execute(BlinkSequence sequence, uint16_t terminal_level) {
  updateSequence(std::move(sequence), 0, terminal_level);
}

void Blinker::set(uint16_t intensity) { updateSequence({}, 0, intensity); }

void Blinker::turnOn() { set(65535); }

void Blinker::turnOff() { set(0); }

void Blinker::updateSequence(BlinkSequence sequence, int repetitions,
                             uint16_t terminal_level) {
  roo::lock_guard<roo::mutex> lock(mutex_);
  sequence_ = std::move(sequence.sequence_);
  terminal_level_ = terminal_level;
  current_level_ = terminal_level_;
  repetitions_ = repetitions;
  fade_in_progress_ = false;
  pos_ = 0;
  if (!sequence_.empty() && !stepper_.is_scheduled()) {
    stepper_.scheduleNow(roo_scheduler::PRIORITY_ELEVATED);
  } else {
    current_level_ = terminal_level_;
    led_.setLevel(current_level_);
  }
}

void Blinker::step() {
  roo::lock_guard<roo::mutex> lock(mutex_);
  if (fade_in_progress_) {
    roo_time::Uptime now = roo_time::Uptime::Now();
    if (now >= fade_end_time_) {
      fade_in_progress_ = false;
      current_level_ = fade_target_level_;
      led_.setLevel(current_level_);
    } else {
      float progress = (now - fade_start_time_).inMillisFloat() /
                       (fade_end_time_ - fade_start_time_).inMillisFloat();
      current_level_ = fade_start_level_ +
                       (fade_target_level_ - fade_start_level_) * progress;
      led_.setLevel(current_level_);
      stepper_.scheduleAfter(roo_time::Millis(20),
                             roo_scheduler::PRIORITY_ELEVATED);
      return;
    }
  }
  uint16_t next_delay = 0;
  do {
    if (pos_ >= sequence_.size()) {
      sequence_.clear();
      current_level_ = terminal_level_;
      led_.setLevel(current_level_);
      return;
    }
    const Step& s = sequence_[pos_];
    switch (s.type_) {
      case Step::kSet: {
        current_level_ = s.target_level_;
        led_.setLevel(current_level_);
        break;
      }
      case Step::kHold: {
        next_delay = s.duration_millis_;
        break;
      }
      case Step::kFade:
      default: {
        if (led_.fade(s.target_level_, roo_time::Millis(s.duration_millis_))) {
          next_delay = s.duration_millis_;
        } else {
          fade_in_progress_ = true;
          fade_start_level_ = current_level_;
          fade_target_level_ = s.target_level_;
          fade_start_time_ = roo_time::Uptime::Now();
          fade_end_time_ =
              fade_start_time_ + roo_time::Millis(s.duration_millis_);
          next_delay = 20;
        }
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

BlinkSequence Blink(roo_time::Duration period, int duty_percent,
                    int rampup_percent_on, int rampup_percent_off) {
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

  BlinkSequence result;

  if (millis_1st_rampup > 0) {
    result.add(FadeOn(Millis(millis_1st_rampup)));
  } else {
    result.add(TurnOn());
  }
  if (millis_1st_rampup < millis_1st) {
    result.add(Hold(Millis(millis_1st - millis_1st_rampup)));
  }

  if (millis_2nd_rampup > 0) {
    result.add(FadeOff(Millis(millis_2nd_rampup)));
  } else {
    result.add(TurnOff());
  }
  if (millis_2nd_rampup < millis_2nd) {
    result.add(Hold(Millis(millis_2nd - millis_2nd_rampup)));
  }

  return result;
}

}  // namespace roo_blink