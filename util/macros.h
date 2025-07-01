#pragma once

#define DISALLOW_COPY_AND_ASSIGN(ClassType)                                    \
  ClassType(ClassType &&other) = delete;                                       \
  ClassType &operator=(ClassType &&other) = delete;                            \
  ClassType(const ClassType &) = delete;                                       \
  const ClassType &operator=(const ClassType &) = delete;
