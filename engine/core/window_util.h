#pragma once

#include <array>

#include "util/macros.h"
#include "util/report/macros.h"
#include "util/time/downsampler.h"
#include "util/time/time.h"

#include "engine/core/types.h"

static constexpr int kDefaultWidth = 800;
static constexpr int kDefaultHeight = 600;
static constexpr size_t kFrameDelta = 60;

namespace gib {

// Struct updated on each window tick with current time and delta time since
// last frame.
struct WindowTick {
  WindowTick(time_util::TimePoint current_time,
             time_util::DurationUsec delta_time)
      : current_time(current_time), delta_time(delta_time) {}

  // Current time.
  time_util::TimePoint current_time;
  // Time in usec since last frame.
  time_util::DurationUsec delta_time;
};

// Track FPS over the last kFrameDelta frames.
class FpsTracker {
public:
  struct FpsStat {
    float avg_fps{0.f};
    size_t frame_count{0};
    float frame_delta_sum{0.f};
    std::array<float, kFrameDelta> frame_deltas{0.f};
  };

  FpsStat fps_stat;

  FpsTracker(const float report_dt) {
    downsampler_.SetDt(report_dt);
    fps_stat.frame_deltas.fill(0.f);
  }

  ~FpsTracker() = default;

  // Average FPS over the sliding window.
  const float GetAvgFps() const {
    const int samples = std::min(fps_stat.frame_count, kFrameDelta);
    if (samples == 0 || fps_stat.frame_delta_sum == 0.f) {
      return 0.f; // nothing recorded yet
    }
    return samples / fps_stat.frame_delta_sum;
  }

  // Call once per rendered frame.
  void Tick(const WindowTick &window_tick) {
    const auto delta_time = time_util::to_seconds<time_util::DurationUsec>(
        time_util::elapsed_usec(last_time_, window_tick.current_time));
    last_time_ = window_tick.current_time;

    const size_t offset = fps_stat.frame_count % kFrameDelta;
    fps_stat.frame_delta_sum += (delta_time - fps_stat.frame_deltas[offset]);
    fps_stat.frame_count++;

    if (downsampler_.UpdateAndCheckIfProcess(window_tick.current_time) ||
        fps_stat.frame_delta_sum < 0) {
      INFO("Avg FPS: {}", GetAvgFps());
    }
    fps_stat.frame_deltas[offset] = delta_time;
  }

  DISALLOW_COPY_AND_ASSIGN(FpsTracker);

private:
  time_util::DownSampler downsampler_;
  time_util::TimePoint last_time_{time_util::now()};
};

} // namespace gib