#include "util/report/macros.h"
#include "util/time/downsampler.h"
#include "util/time/time.h"

static constexpr int kDefaultWidth = 800;
static constexpr int kDefaultHeight = 600;
static constexpr int kFrameDelta = 120;

namespace core {

struct WindowSize {
  int height{0};
  int width{0};
};

struct WindowTick {
  WindowTick(time_util::TimePoint current_time,
             time_util::DurationUsec delta_time)
      : current_time(current_time), delta_time(delta_time) {}

  time_util::TimePoint current_time;
  time_util::DurationUsec delta_time;
};

struct FpsStat {
  float avg_fps{0.f};
  int frame_count{0};
  float frame_delta_sum{0.f};
  std::array<float, kFrameDelta> frame_deltas;
  int frame_delta{kFrameDelta};
};

// Track FPS over the last kFrameDelta frames.
struct FpsTracker {
  time_util::DownSampler downsampler_;
  time_util::TimePoint last_time_{time_util::now()};
  FpsStat fps_stat;

  explicit FpsTracker(float report_dt) { downsampler_.SetDt(report_dt); }

  // Average FPS over the sliding window.
  const float GetAvgFps() const {
    const int samples = std::min(fps_stat.frame_count, kFrameDelta);
    if (samples == 0 || fps_stat.frame_delta_sum == 0.f) {
      return 0.f; // nothing recorded yet
    }
    return (1'000'000.f * samples) /
           static_cast<float>(fps_stat.frame_delta_sum);
  }

  const FpsStat GetFpsStats() const {
    return FpsStat{GetAvgFps(), fps_stat.frame_count, fps_stat.frame_delta_sum,
                   fps_stat.frame_deltas, fps_stat.frame_delta};
  }

  // Call once per rendered frame.
  void Tick(const WindowTick &window_tick) {
    const auto delta_time = time_util::to_seconds(
        time_util::elapsed_usec(last_time_, window_tick.current_time));
    last_time_ = window_tick.current_time;

    // Ring-buffer update.
    const unsigned int offset = fps_stat.frame_count % kFrameDelta;
    fps_stat.frame_delta_sum += delta_time - fps_stat.frame_deltas[offset];
    fps_stat.frame_deltas[offset] = delta_time;

    ++fps_stat.frame_count;

    if (downsampler_.UpdateAndCheckIfProcess(window_tick.current_time)) {
      INFO("Avg FPS: {}", GetAvgFps());
    }
  }
};

} // namespace core