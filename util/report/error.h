#pragma once

#include "util/report/macros.h"

#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"

static std::string GlErrorToString(GLenum error) {
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

#define CHECK_GL_ERROR()                                                       \
  do {                                                                         \
    GLenum error = glGetError();                                               \
    ASSERT(error == GL_NO_ERROR, "ERROR::GL::{}", GlErrorToString(error));     \
  } while (0)