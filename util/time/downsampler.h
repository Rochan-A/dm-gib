#pragma once

#include <chrono>
#include <cstdint>

#include "util/time/time.h"

namespace time_util {

class DownSampler {
public:
  DownSampler() = default;

  explicit DownSampler(const float dt_s, const bool skip_first_call = false) {
    Reset(dt_s, skip_first_call);
  }

  ~DownSampler() = default;

  void Reset(const float dt_s, const bool skip_first_call = false) {
    processing_dt_ = seconds_to_usec(dt_s);
    skip_first_call_ = skip_first_call;
    initialized_ = false;
  }

  void Reset() { Reset(GetProcessingDt(), skip_first_call_); }
  void SetDt(const float dt_s) { processing_dt_ = seconds_to_usec(dt_s); }

  void Update(const TimePoint &current_tp,
              const bool allow_initialization = false) {
    if (!initialized_) {
      if (allow_initialization) {
        initialized_ = true;
        last_process_ = current_tp;
        next_process_ = current_tp + processing_dt_;
      }
      return;
    }
    last_process_ = current_tp;
    next_process_ = current_tp + processing_dt_;
  }

  // Fast-path: update & decide.  Returns true if we should “process”.
  bool UpdateAndCheckIfProcess(const TimePoint &current_tp) {
    if (processing_dt_.count() < 0) {
      return false;
    }

    if (!initialized_) {
      initialized_ = true;
      last_process_ = current_tp;
      next_process_ = current_tp + processing_dt_;
      return !skip_first_call_;
    }

    if (current_tp < next_process_) {
      return false;
    }

    last_process_ = current_tp;
    next_process_ = current_tp + processing_dt_;
    return true;
  }

  float GetProcessingDt() const { return to_seconds(processing_dt_); }
  bool Initialized() const { return initialized_; }
  bool GetSkipFirstTime() const { return skip_first_call_; }
  TimePoint LastTimeProcessed() const { return last_process_; }

private:
  DurationUsec processing_dt_{DurationUsec::zero()};
  TimePoint last_process_{};
  TimePoint next_process_{};
  bool initialized_{false};
  bool skip_first_call_{false};
};

} // namespace time_util