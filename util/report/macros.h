#pragma once

#include <chrono>
#include <ctime>
#include <exception>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <string>

#include <cstring>
#include <execinfo.h>

#include "util/time/time.h"

namespace logging {

// Static initialization for program start time.
static const auto kProgramStart = time_util::now();

// Returns the elapsed time (in seconds) since the program started.
inline const float MonotonicTimeSeconds() {
  return time_util::to_seconds<time_util::DurationUsec>(
      time_util::elapsed_usec(kProgramStart));
}

// Returns a stack trace.
inline const std::string GetStackTrace(const int skip_frames = 2) {
  static const int kMaxFrames = 64;
  void *addr_list[kMaxFrames + 1];

  int addr_len = backtrace(addr_list, kMaxFrames);
  if (addr_len == 0) {
    return "  <no stack trace available>";
  }

  char **symbol_list = backtrace_symbols(addr_list, addr_len);
  if (symbol_list == nullptr) {
    return "  <backtrace_symbols failed>";
  }

  std::string result;
  // Skip the first few frames, which typically include this function and its
  // caller.
  for (int i = skip_frames; i < addr_len; ++i) {
    result += fmt::format("  {}\n", symbol_list[i]);
  }

  free(symbol_list);
  return result;
}

// Returns the current system time as a string, i.e., "2025-04-13T02:03:43".
inline std::string CurrentSystemTimeString() {
  using SystemClock = std::chrono::system_clock;
  auto now = SystemClock::now();
  std::time_t now_c = SystemClock::to_time_t(now);

  char buf[100];
  std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&now_c));
  return {buf};
}

// Throws a std::runtime_error with a stack trace.
inline void ThrowFatalError(const char *file, int line, const char *func,
                            const std::string &msg) {
  std::string stacktrace = GetStackTrace(2);
  std::string const full_message =
      fmt::format("FATAL ERROR at {}:{} in {}:\n{}\nStack trace:\n{}", file,
                  line, func, msg, stacktrace);

  throw std::runtime_error(full_message);
}

// ANSI colours (bright so they show up on light or dark themes)
namespace ansi {
constexpr const char *kReset = "\x1b[0m";
constexpr const char *kBlue = "\x1b[1;34m";
constexpr const char *kYellow = "\x1b[1;33m";
constexpr const char *kGreen = "\x1b[1;32m";
constexpr const char *kRed = "\x1b[1;31m";
} // namespace ansi

inline void LogMessage(const char *level, const char *file, const int line,
                       const char *func, const std::string &msg) {
  const char *color = (level[0] == 'I')   ? ansi::kBlue
                      : (level[0] == 'W') ? ansi::kYellow
                      : (level[0] == 'D') ? ansi::kGreen
                      : (level[0] == 'A') ? ansi::kRed
                                          : ansi::kReset;

  std::cerr << fmt::format("{}{} {} [{:11.6f}] {}:{} {}{}\n", color, level,
                           CurrentSystemTimeString(), MonotonicTimeSeconds(),
                           file, line, msg, ansi::kReset);
}

} // namespace logging

// kdebug_break  (works on GCC/Clang/MSVC; harmless on others)
#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define kdebug_break() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define kdebug_break() __debugbreak()
#endif
#endif
#if !defined(kdebug_break)
#if defined(__clang__) || defined(__gcc__)
#define kdebug_break() __builtin_trap()
#elif defined(_MSC_VER)
#include <intrin.h>
#define kdebug_break() __debugbreak()
#else
#define kdebug_break() ((void)0)
#endif
#endif

#define DEBUG(msg, ...)                                                        \
  ::logging::LogMessage("D", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define WARNING(msg, ...)                                                      \
  ::logging::LogMessage("W", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define INFO(msg, ...)                                                         \
  ::logging::LogMessage("I", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define THROW_FATAL(msg, ...)                                                  \
  ::logging::ThrowFatalError(__FILE__, __LINE__, __func__,                     \
                             fmt::format(msg, ##__VA_ARGS__))

#define ASSERT(expr, msg, ...)                                                 \
  do {                                                                         \
    if (!(expr)) {                                                             \
      ::logging::LogMessage("A", __FILE__, __LINE__, __func__,                 \
                            fmt::format("Assertion failed: {}. {}", #expr,     \
                                        fmt::format(msg, ##__VA_ARGS__)));     \
      kdebug_break();                                                          \
      ::logging::ThrowFatalError(                                              \
          __FILE__, __LINE__, __func__,                                        \
          fmt::format("Assertion failed: {}. {}", #expr,                       \
                      fmt::format(msg, ##__VA_ARGS__)));                       \
    }                                                                          \
  } while (0)