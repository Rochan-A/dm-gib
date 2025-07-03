#define GLAD_GL_IMPLEMENTATION
#include "third_party/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace gib {

// TODO
class TextureManager {
public:
  TextureManager() {}

  void GenTextures(GLsizei n, GLuint texture, GLenum bind_target) {
    glGenTextures(n, &texture);
    glBindTexture(bind_target, texture);
  }

private:
};

} // namespace gib