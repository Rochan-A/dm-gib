#pragma once

#include <cstdint>

namespace time_util {

typedef int64_t usec_t;

constexpr inline usec_t SecondsToUsec(float s) {
  return static_cast<usec_t>(s * 1'000'000.f);
}
constexpr inline float UsecToSecondsf(usec_t u) {
  return static_cast<float>(u) * 1e-6f;
}

class DownSampler {
public:
  DownSampler() = default;

  explicit DownSampler(float dt_s, bool skip_first_call = false) {
    Reset(dt_s, skip_first_call);
  }

  ~DownSampler() = default;

  // Restart with a new dt_s.
  void Reset(float dt_s, bool skip_first_call = false) {
    processing_dt_ = SecondsToUsec(dt_s);
    skip_first_call_ = skip_first_call;
    initialized_ = false;
    usec_last_process_ = 0;
    usec_next_process_ = 0;
  }

  // Restart but keep the same dt_s.
  void Reset() { Reset(GetProcessingDt(), skip_first_call_); }

  // Change dt_s in-place.
  void SetDt(float dt_s) { processing_dt_ = SecondsToUsec(dt_s); }

  // Update only (optionally boot-strapping the first time).
  void Update(usec_t current_usec, bool allow_initialization = false) {
    if (!initialized_) {
      if (allow_initialization) {
        initialized_ = true;
        usec_last_process_ = current_usec;
        usec_next_process_ = current_usec + processing_dt_;
      }
      return;
    }
    usec_last_process_ = current_usec;
    usec_next_process_ = current_usec + processing_dt_;
  }

  // Fast path: update & decide. Returns true if we should “process”.
  bool UpdateAndCheckIfProcess(usec_t current_usec) {
    if (processing_dt_ < 0) {
      return false; // disabled
    }

    if (!initialized_) { // first ever call
      initialized_ = true;
      usec_last_process_ = current_usec;
      usec_next_process_ = current_usec + processing_dt_;
      return !skip_first_call_;
    }

    if (current_usec < usec_next_process_) {
      return false;
    }

    usec_last_process_ = current_usec;
    usec_next_process_ = current_usec + processing_dt_;
    return true;
  }

  // Read-only check (cheap) – does *not* move internal state.
  bool CheckIfProcess(usec_t current_usec, bool allow_initialization = false) {
    if (processing_dt_ < 0)
      return false;

    if (!initialized_) {
      if (allow_initialization) {
        initialized_ = true;
        usec_last_process_ = current_usec;
        usec_next_process_ = current_usec + processing_dt_;
      }
      return !skip_first_call_ && allow_initialization;
    }
    return current_usec >= usec_next_process_;
  }

  float GetProcessingDt() const { return UsecToSecondsf(processing_dt_); }
  bool Initialized() const { return initialized_; }
  bool GetSkipFirstTime() const { return skip_first_call_; }
  usec_t LastusecProcessed() const { return usec_last_process_; }

private:
  usec_t processing_dt_{0};     // desired dt (usec)
  usec_t usec_last_process_{0}; // last time we returned true
  usec_t usec_next_process_{0}; // deadline for next “true”
  bool initialized_{false};
  bool skip_first_call_{false};
};

} // namespace time_util