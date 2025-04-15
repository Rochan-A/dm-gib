#pragma once

#include <chrono>
#include <ctime>
#include <exception>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <cstring>
#include <execinfo.h>

namespace logging {

// A static initialization for program start time. This ensures it's set once.
static const auto kProgramStart = std::chrono::steady_clock::now();

// Returns the elapsed time (in seconds) since the program started.
inline const double MonotonicTimeSeconds() {
  auto diff = std::chrono::steady_clock::now() - kProgramStart;
  return std::chrono::duration<double>(diff).count();
}

// Returns a stack trace.
inline const std::string GetStackTrace(int skip_frames = 2) {
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

// Returns the current system time as a string, i.e., "2025-04-13 02:03:43".
inline std::string CurrentSystemTimeString() {
  using SystemClock = std::chrono::system_clock;
  auto now = SystemClock::now();
  std::time_t now_c = SystemClock::to_time_t(now);

  char buf[100];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
  return {buf};
}

// Logs a message with color coding based on the level.
inline void LogMessage(const char *level, const char *file, int line,
                       const char *func, const std::string &msg) {
  std::string system_time = CurrentSystemTimeString();
  double monotonic_time = MonotonicTimeSeconds();

  // Pick color by level.
  std::string color_code;
  if (std::strcmp(level, "INFO") == 0) {
    color_code = "\033[34m"; // Blue
  } else if (std::strcmp(level, "WARNING") == 0) {
    color_code = "\033[33m"; // Yellow
  } else if (std::strcmp(level, "DEBUG") == 0) {
    color_code = "\033[32m"; // Green
  } else {
    color_code = "\033[0m"; // Default
  }

  std::cerr << fmt::format("{}[{}] {} | {:.9f}s | {}:{} ({}) - {}\033[0m\n",
                           color_code, level, system_time, monotonic_time, file,
                           line, func, msg);
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

} // namespace logging

// Always define kdebug_break in case it is ever needed outside assertions (i.e
// fatal log errors) Try via __has_builtin first.
#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define kdebug_break() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define kdebug_break() __debugbreak()
#endif
#endif

// If not setup, try the old way.
#if !defined(kdebug_break)
#if defined(__clang__) || defined(__gcc__)
/** @brief Causes a debug breakpoint to be hit. */
#define kdebug_break() __builtin_trap()
#elif defined(_MSC_VER)
#include <intrin.h>
/** @brief Causes a debug breakpoint to be hit. */
#define kdebug_break() __debugbreak()
#else
// Fall back to x86/x86_64
#define kdebug_break() asm { int 3 }
#endif
#endif

#define DEBUG(msg, ...)                                                        \
  ::logging::LogMessage("D", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define INFO(msg, ...)                                                         \
  ::logging::LogMessage("I", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define WARNING(msg, ...)                                                      \
  ::logging::LogMessage("W", __FILE__, __LINE__, __func__,                     \
                        fmt::format(msg, ##__VA_ARGS__))

#define THROW_FATAL(msg, ...)                                                  \
  ::logging::ThrowFatalError(__FILE__, __LINE__, __func__,                     \
                             fmt::format(msg, ##__VA_ARGS__))

#define ASSERT(expr, msg, ...)                                                 \
  do {                                                                         \
    if (!(expr)) {                                                             \
      ::logging::ThrowFatalError(                                              \
          __FILE__, __LINE__, __func__,                                        \
          fmt::format("Assertion failed: {}. Message: {}", #expr,              \
                      fmt::format(msg, ##__VA_ARGS__)));                       \
      kdebug_break();                                                          \
    }                                                                          \
  } while (0)
