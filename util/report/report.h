#pragma once

#include <chrono>
#include <ctime>
#include <exception>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <string>

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

#include <cstring>
#include <execinfo.h>

#include "util/time/time.h"

namespace report {

inline std::string GlErrorToString(const GLenum &error) {
  switch (error) {
  case GL_NO_ERROR:
    return "NO_ERROR";
  case GL_INVALID_ENUM:
    return "INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "INVALID_OPERATION";
  case GL_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  case GL_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case GL_OUT_OF_MEMORY:
    return "OUT_OF_MEMORY";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "INVALID_FRAMEBUFFER_OPERATION";
  default:
    return "UNKNOWN";
  }
}

// Static initialization for program start time.
static const auto kProgramStart = time_util::now();

// Returns the elapsed time (in seconds) since the program started.
inline float MonotonicTimeSeconds() {
  return time_util::to_seconds<time_util::DurationUsec>(
      time_util::elapsed_usec(kProgramStart));
}

// Returns a stack trace.
inline std::string GetStackTrace(const int skip_frames = 2) {
  static const int kMaxFrames = 64;
  void *addr_list[kMaxFrames + 1];

  int const addr_len = backtrace(addr_list, kMaxFrames);
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
  std::time_t const now_c = SystemClock::to_time_t(now);

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
                       const char * /*func*/, const std::string &msg) {
  const char *color = (level[0] == 'I')   ? ansi::kBlue
                      : (level[0] == 'W') ? ansi::kYellow
                      : (level[0] == 'D') ? ansi::kGreen
                      : (level[0] == 'A') ? ansi::kRed
                                          : ansi::kReset;

  std::cerr << fmt::format("{}{} {} [{:11.6f}] {}:{} {}{}\n", color, level,
                           CurrentSystemTimeString(), MonotonicTimeSeconds(),
                           file, line, msg, ansi::kReset);
}

} // namespace report

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
  report::LogMessage("D", __FILE__, __LINE__, __func__,                        \
                     fmt::format(msg, ##__VA_ARGS__))

#define WARNING(msg, ...)                                                      \
  report::LogMessage("W", __FILE__, __LINE__, __func__,                        \
                     fmt::format(msg, ##__VA_ARGS__))

#define INFO(msg, ...)                                                         \
  report::LogMessage("I", __FILE__, __LINE__, __func__,                        \
                     fmt::format(msg, ##__VA_ARGS__))

#define THROW_FATAL(msg, ...)                                                  \
  report::ThrowFatalError(__FILE__, __LINE__, __func__,                        \
                          fmt::format(msg, ##__VA_ARGS__))

#define ASSERT(expr, msg, ...)                                                 \
  do {                                                                         \
    if (!(expr)) {                                                             \
      report::LogMessage("A", __FILE__, __LINE__, __func__,                    \
                         fmt::format("Assertion failed: {}. {}", #expr,        \
                                     fmt::format(msg, ##__VA_ARGS__)));        \
      kdebug_break();                                                          \
      report::ThrowFatalError(__FILE__, __LINE__, __func__,                    \
                              fmt::format("Assertion failed: {}. {}", #expr,   \
                                          fmt::format(msg, ##__VA_ARGS__)));   \
    }                                                                          \
  } while (0)

#define CHECK_GL_ERROR()                                                       \
  do {                                                                         \
    GLenum error = glGetError();                                               \
    ASSERT(error == GL_NO_ERROR, "ERROR::GL::{}",                              \
           report::GlErrorToString(error));                                    \
  } while (0)

static constexpr int kMaxLogLength = 1024;

#define CHECK_GL_PROGRAM_ERROR(success, get_log_func, log_func_arg)            \
  do {                                                                         \
    if (static_cast<int>(success) == 0) {                                      \
      char error[kMaxLogLength];                                               \
      get_log_func(log_func_arg, kMaxLogLength, nullptr, error);               \
      report::ThrowFatalError(                                                 \
          __FILE__, __LINE__, __func__,                                        \
          fmt::format("Failed to link program: {}", error));                   \
    }                                                                          \
  } while (0)