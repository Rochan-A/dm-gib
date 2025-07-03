#include "engine/core/types.h"
#include "engine/core/window.h"
#include "engine/core/window_util.h"
#include "engine/shaders/shader.h"
#include "third_party/concise_args/ConciseArgs.h"

#include "engine/shaders/types.h"
#include "engine/vertex_util/vertex_array.h"
#include "gib/window.h"
#include "third_party/glm/ext/matrix_clip_space.hpp"
#include "third_party/glm/ext/matrix_transform.hpp"
#include "third_party/glm/fwd.hpp"
#include "third_party/glm/matrix.hpp"

#include "util/report/error.h"
#include <functional>
#include <memory>

#include "util/report/macros.h"

#include "engine/core/types.h"
#include "engine/shaders/shader.h"
#include "third_party/glm/gtc/type_ptr.hpp"

struct Vertex {
  glm::vec2 pos;
  glm::vec3 col;
};

static const char *vertex_shader_text =
    "#version 330\n"
    "uniform mat4 MVP;\n"
    "in vec3 vCol;\n"
    "in vec2 vPos;\n"
    "out vec3 color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
    "    color = vCol;\n"
    "}\n";

static const char *fragment_shader_text = "#version 330\n"
                                          "in vec3 color;\n"
                                          "out vec4 fragment;\n"
                                          "void main()\n"
                                          "{\n"
                                          "    fragment = vec4(color, 1.0);\n"
                                          "}\n";

static const Vertex vertices[3] = {{{-0.6f, -0.4f}, {1.f, 0.f, 0.f}},
                                   {{0.6f, -0.4f}, {0.f, 1.f, 0.f}},
                                   {{0.f, 0.6f}, {0.f, 0.f, 1.f}}};

namespace gib {

class Triangle final : public BaseWindow<Triangle> {
public:
  Triangle() : BaseWindow("Triangle") {
    ToggleMouseButtonInput(true);
    ToggleKeyInput(true);
    ToggleScrollInput(true);
    ToggleMouseMoveInput(true);

    SetGLFWInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    SetMouseButtonBehavior(MouseButtonBehavior::NONE);
    SetEscKeyBehavior(EscBehavior::NONE);

    ShaderSource vertex_shader(vertex_shader_text, ShaderType::VERTEX, false);
    ShaderSource fragment_shader(fragment_shader_text, ShaderType::FRAGMENT,
                                 false);

    Shader shader{vertex_shader, fragment_shader};
    shader.Link();

    program = shader.GetProgramId();

    mvp_location = glGetUniformLocation(program, "MVP");
    // const GLint vpos_location = glGetAttribLocation(program, "vPos");
    // const GLint vcol_location = glGetAttribLocation(program, "vCol");

    VertexArray vertex_array;
    vertex_array.AddVertexBuffer(&vertices, sizeof(vertices));
    vertex_array.AddVertexAttribute(2, GL_FLOAT, false);
    vertex_array.AddVertexAttribute(3, GL_FLOAT, false);
    vertex_array.Apply();

    vertex_array_vbo = vertex_array.GetVbo();
  }

  void Tick(const Tick &tick, GlfwWindow &window) {
    const float ratio = window.GetAspectRatio();

    glm::mat4x4 m, p, mvp;
    m = glm::identity<glm::mat4x4>();
    m = glm::rotate(m, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
    p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);

    mvp = p * m;
    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(vertex_array_vbo);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  void Tock(const struct Tick &tick, GlfwWindow &window) {}

  void DebugUI(GlfwWindow &window) {}

private:
  unsigned int program;
  GLint mvp_location;
  unsigned int vertex_array_vbo;
};
} // namespace gib

int main(int argc, char **argv) {
  bool fullscreen = false;
  bool debug = false;

  ConciseArgs parser(argc, argv);
  parser.add<bool>(fullscreen, "f", "fullscreen", "Launch in full screen");
  parser.add<bool>(debug, "d", "debug", "Enable debug window");
  parser.parse();
  DEBUG("Fullscreen {}, debug {}", fullscreen, debug);

  gib::Triangle triangle;
  triangle.Run();

  return 0;
}
