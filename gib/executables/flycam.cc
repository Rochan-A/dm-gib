#include "engine/core/frame_util.h"
#include "engine/core/gl_window.h"
#include "engine/core/types.h"
#include "engine/shaders/shader.h"
#include "engine/vertex_util/vertex_array.h"

#include "gib/window.h"

#include "third_party/glm/ext/matrix_clip_space.hpp"
#include "third_party/glm/ext/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"

#include "engine/camera/camera_base.h"
#include "engine/camera/fly_camera.h"
#include "engine/core/input.h"
#include "util/time/time.h"

struct FloorVertex {
  glm::vec3 pos;
  glm::vec3 col;
};

// Two triangles forming a 10×10 plane on the XZ axis
static const FloorVertex floor_vertices[] = {
    {{-5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}},
    {{5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}},
    {{5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}},

    {{-5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}},
    {{5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}},
    {{-5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}}};

static const char *vertex_shader_src = R"GLSL(
#version 330 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 MVP;

out vec3 color;

void main() {
  gl_Position = MVP * vec4(vPos, 1.0);
  color = vCol;
}
)GLSL";

static const char *fragment_shader_src = R"GLSL(
#version 330 core
in vec3 color;
out vec4 frag;
void main() { frag = vec4(color, 1.0); }
)GLSL";

namespace gib {

class FlyCamDemo final : public WindowBase<FlyCamDemo> {
public:
  FlyCamDemo() : WindowBase("FlyCam Demo"), camera_(glm::vec3(0.f, 2.f, 5.f)) {
    ToggleMouseButtonInput(true);
    ToggleKeyInput(true);
    ToggleScrollInput(true);
    ToggleMouseMoveInput(true);

    // Capture mouse cursor for FPS‑style look‑around
    SetGLFWInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    SetMouseButtonBehavior(MouseButtonBehavior::NONE);
    SetEscKeyBehavior(EscBehavior::TOGGLE_MOUSE_CAPTURE);

    // Build shader
    ShaderSource vs(vertex_shader_src, ShaderType::VERTEX, false);
    ShaderSource fs(fragment_shader_src, ShaderType::FRAGMENT, false);
    Shader shader{vs, fs};
    shader.Link();
    program_ = shader.GetProgramId();
    mvp_loc_ = glGetUniformLocation(program_, "MVP");

    // Build floor VAO
    VertexArray vao;
    vao.AddVertexBuffer(&floor_vertices, sizeof(floor_vertices));
    vao.AddVertexAttribute(3, GL_FLOAT, false); // position
    vao.AddVertexAttribute(3, GL_FLOAT, false); // color
    vao.Apply();
    floor_vao_ = vao.GetVbo();
  }

  void Tick(const FrameTick &frame_tick, GlfwWindow &window) {
    camera_.Tick(frame_tick);
    float dt =
        time_util::to_seconds<time_util::DurationUsec>(frame_tick.delta_time);
    ProcessInput(dt);

    glm::mat4 view = camera_.GetViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(camera_.Fov()),
                                      window.GetAspectRatio(), 0.1f, 100.0f);
    glm::mat4 model = glm::identity<glm::mat4>();
    glm::mat4 mvp = proj * view * model;

    glUseProgram(program_);
    glUniformMatrix4fv(mvp_loc_, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(floor_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  void Tock(const struct FrameTick &tick, GlfwWindow &window) {}

  void DebugUI(GlfwWindow &window) { camera_.DebugUI(); }

private:
  /*================ Input Handling ================*/
  void ProcessInput(float dt) {
    Input input = GetInput();
    // WASD + Space + Shift
    if (input.key_state[GLFW_KEY_W] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_W] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::FORWARD, dt);
    if (input.key_state[GLFW_KEY_S] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_S] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::BACKWARD, dt);
    if (input.key_state[GLFW_KEY_A] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_A] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::LEFT, dt);
    if (input.key_state[GLFW_KEY_D] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_D] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::RIGHT, dt);
    if (input.key_state[GLFW_KEY_SPACE] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_SPACE] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::UP, dt);
    if (input.key_state[GLFW_KEY_LEFT_SHIFT] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_LEFT_SHIFT] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::DOWN, dt);
    if (input.key_state[GLFW_KEY_Q] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_Q] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::UP, dt);
    if (input.key_state[GLFW_KEY_E] == KeyAction::PRESS ||
        input.key_state[GLFW_KEY_E] == KeyAction::REPEAT)
      camera_.ProcessKeyboard(Directions::DOWN, dt);

    // Mouse look — first frame just sets baseline
    if (first_mouse_) {
      last_mouse_pos_ = input.mouse_pos;
      first_mouse_ = false;
    }
    if (ctx_.enable_mouse_capture) {
      Offset delta = input.mouse_pos - last_mouse_pos_;
      last_mouse_pos_ = input.mouse_pos;
      camera_.ProcessMouseMovement(delta.x, -delta.y); // invert Y
      // Mouse wheel zoom
      if (input.scroll_offset.y != 0.0f) {
        camera_.ProcessMouseScroll(input.scroll_offset.y);
        input.scroll_offset = {0.f, 0.f};
      }
    }
  }

  unsigned int program_ = 0;
  GLint mvp_loc_ = -1;
  unsigned int floor_vao_ = 0;

  FlyCameraModel camera_;

  Offset last_mouse_pos_{0.f, 0.f};
  bool first_mouse_ = true;
};

} // namespace gib

int main(int argc, char **argv) {
  gib::FlyCamDemo demo;
  demo.Run();

  return 0;
}
