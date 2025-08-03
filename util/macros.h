#pragma once

#include <tracy/Tracy.hpp>

#define DISALLOW_COPY_AND_ASSIGN(ClassType)                                    \
  ClassType(ClassType &&other) = delete;                                       \
  ClassType &operator=(ClassType &&other) = delete;                            \
  ClassType(const ClassType &) = delete;                                       \
  const ClassType &operator=(const ClassType &) = delete;

#define PROFILE_FRAME(name) FrameMarkNamed(name)
#define PROFILE_SCOPE_N(name) ZoneScopedN(name)
#define PROFILE_LOG(text) TracyMessage(text)
#define PROFILE_LOG_STR(text) TracyMessage(text.c_str(), text.size())
#define PROFILE_VALUE(text, value) TracyPlot(text, value)