#pragma once

#include <chrono>
#include <ctime>

#include <sys/types.h>

namespace time_util {

typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::nanoseconds DurationNsec;
typedef std::chrono::microseconds DurationUsec;
typedef std::chrono::seconds DurationSec;

inline const TimePoint now() noexcept {
  return std::chrono::steady_clock::now();
}

constexpr inline DurationNsec to_nsec(DurationUsec d) noexcept {
  return std::chrono::duration_cast<DurationNsec>(d);
}

constexpr inline DurationUsec to_usec(DurationNsec d) noexcept {
  return std::chrono::duration_cast<DurationUsec>(d);
}

template <class Duration>
constexpr inline double to_seconds(Duration d) noexcept {
  return std::chrono::duration<double>(d).count();
}

inline DurationNsec elapsed_nsec(TimePoint start,
                                 TimePoint end = now()) noexcept {
  return std::chrono::duration_cast<DurationNsec>(end - start);
}

inline DurationUsec elapsed_usec(TimePoint start,
                                 TimePoint end = now()) noexcept {
  return std::chrono::duration_cast<DurationUsec>(end - start);
}

inline timespec to_timespec(DurationNsec d) noexcept {
  timespec ts;
  ts.tv_sec = static_cast<time_t>(d.count() / 1'000'000'000);
  ts.tv_nsec = static_cast<long>(d.count() % 1'000'000'000);
  return ts;
}

inline timeval to_timeval(DurationUsec d) noexcept {
  timeval tv;
  tv.tv_sec = static_cast<time_t>(d.count() / 1'000'000);
  tv.tv_usec = static_cast<suseconds_t>(d.count() % 1'000'000);
  return tv;
}

inline DurationNsec from_timespec(const timespec &ts) noexcept {
  return std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec);
}

inline DurationUsec from_timeval(const timeval &tv) noexcept {
  return std::chrono::seconds(tv.tv_sec) +
         std::chrono::microseconds(tv.tv_usec);
}

} // namespace time_util
