#include "engine/shaders/compiler.h"
#include "engine/shaders/types.h"

namespace gib {

unsigned int ShaderCompiler::LoadAndCompile(const ShaderSource &source) {
  std::stringstream shader_stream;
  if (source.is_path) {
    try {
      std::ifstream shader_file;
      shader_file.open(source.shader);
      shader_stream << shader_file.rdbuf();
      shader_file.close();
    } catch (const std::ifstream::failure &exc) {
      THROW_FATAL("No support for ShaderSource from path!\n{}",
                  std::string(source.shader));
    }
  }
  ShaderSource const loaded_shader{source.is_path ? shader_stream.str().c_str()
                                                  : source.shader,
                                   source.type, false};
  unsigned int const shader = Compile(loaded_shader, source.type);
  shaders_.push_back(shader);
  return shader;
}

unsigned int ShaderCompiler::Link() {
  int success = false;
  unsigned int const shader_program = glCreateProgram();
  for (const auto &shader : shaders_) {
    glAttachShader(shader_program, shader);
  }
  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  CHECK_GL_PROGRAM_ERROR(success, glGetProgramInfoLog, shader_program);

  for (const auto &shader : shaders_) {
    glDeleteShader(shader);
  }
  shaders_.clear();
  return shader_program;
}

unsigned int ShaderCompiler::Compile(const ShaderSource &source,
                                     const ShaderType type) {
  unsigned int const shader = glCreateShader(static_cast<GLenum>(type));

  glShaderSource(shader, 1, &source.shader, nullptr);
  glCompileShader(shader);

  GLint success = false;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  CHECK_GL_PROGRAM_ERROR(success, glGetShaderInfoLog, shader);

  return shader;
}

} // namespace gib