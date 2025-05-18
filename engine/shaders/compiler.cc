#include "engine/shaders/compiler.h"
#include "engine/shaders/types.h"

namespace gib {

unsigned int ShaderCompiler::LoadAndCompile(const ShaderSource &source) {
  ShaderSource source_loaded(LoadShader(source), source.type, false);
  unsigned int shader = Compile(source_loaded, source.type);
  shaders_.push_back(shader);
  return shader;
}

unsigned int ShaderCompiler::Link() {
  int success;
  unsigned int shader_program = glCreateProgram();
  for (const auto &shader : shaders_) {
    glAttachShader(shader_program, shader);
  }
  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    char error[512];
    glGetProgramInfoLog(shader_program, 512, nullptr, error);
    THROW_FATAL("Failed to link shader(s) (len: {})!\n{}", shaders_.size(),
                error);
  }

  for (const auto &shader : shaders_) {
    glDeleteShader(shader);
  }
  shaders_.clear();
  return shader_program;
}

unsigned int ShaderCompiler::Compile(const ShaderSource &source,
                                     const ShaderType type) {
  GLint success = false;
  unsigned int shader = glCreateShader((GLenum)type);

  glShaderSource(shader, 1, &source.shader, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    char error[512];
    glGetShaderInfoLog(shader, 512, nullptr, error);
    THROW_FATAL("Compiling shader {} (type: {}) failed!\n{}",
                std::string(source.shader), ENUM_TO_STR(type), error);
  }
  return shader;
}

} // namespace gib