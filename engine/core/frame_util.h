#pragma once

#include <array>

#include "util/macros.h"
#include "util/report/report.h"
#include "util/time/downsampler.h"
#include "util/time/time.h"

#include "engine/core/types.h"

#include "third_party/imgui/imgui.h"
#include <cstddef>

static constexpr int kDefaultWidth = 800;
static constexpr int kDefaultHeight = 600;
static constexpr size_t kFrameDelta = 120;

namespace gib {

// Struct updated on each window tick with current time and delta time since
// last frame.
struct FrameTick {
  FrameTick(const time_util::TimePoint current_time,
            const time_util::DurationUsec delta_time)
      : current_time(current_time), delta_time(delta_time) {}

  // Current time.
  const time_util::TimePoint current_time;
  // Time in usec since last frame.
  const time_util::DurationUsec delta_time;
};

// Track FPS over the last kFrameDelta frames.
class FpsTracker {
public:
  explicit FpsTracker(const float report_dt) {
    downsampler_.SetDt(report_dt);
    fps_stat_.frame_deltas.fill(0.f);
  }
  ~FpsTracker() = default;

  // Average FPS over the sliding window.
  [[nodiscard]] float GetAvgFps() const {
    const auto samples =
        static_cast<int>(std::min(fps_stat_.frame_count, kFrameDelta));
    if (samples == 0 || fps_stat_.frame_delta_sum == 0.f) {
      return 0.f; // nothing recorded yet
    }
    return static_cast<float>(samples) / fps_stat_.frame_delta_sum;
  }

  // Call once per rendered frame.
  void Tick(const FrameTick &frame_tick) {
    const auto delta_time = time_util::to_seconds<time_util::DurationUsec>(
        time_util::elapsed_usec(last_time_, frame_tick.current_time));
    last_time_ = frame_tick.current_time;

    const size_t offset = fps_stat_.frame_count % kFrameDelta;
    fps_stat_.frame_delta_sum += (delta_time - fps_stat_.frame_deltas[offset]);
    fps_stat_.frame_count++;

    if (downsampler_.UpdateAndCheckIfProcess(frame_tick.current_time) ||
        fps_stat_.frame_delta_sum < 0) {
      DEBUG("Avg FPS: {}", GetAvgFps());
    }
    fps_stat_.frame_deltas[offset] = delta_time;
  }

  void DebugUI() {
    if (ImGui::CollapsingHeader("FPS", ImGuiTreeNodeFlags_DefaultOpen)) {

      ImGui::Text("Avg FPS: %.2f", GetAvgFps());
      float frame_dt_s = 0.0f;
      const auto &stat = fps_stat_;
      if (!stat.frame_deltas.empty()) {
        size_t const idx = stat.frame_count % stat.frame_deltas.size();
        frame_dt_s = stat.frame_deltas[idx];
      }

      ImGui::SameLine();
      ImGui::Text("Frame dt: %.3f ms", 1e3f * frame_dt_s);
    }
  }

  DISALLOW_COPY_AND_ASSIGN(FpsTracker);

private:
  time_util::DownSampler downsampler_;
  time_util::TimePoint last_time_{time_util::now()};

  struct FpsStat {
    float avg_fps{0.f};
    size_t frame_count{0};
    float frame_delta_sum{0.f};
    std::array<float, kFrameDelta> frame_deltas{0.f};
  };

  FpsStat fps_stat_;
};

} // namespace gib