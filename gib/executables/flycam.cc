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
#include <OpenGL/OpenGL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image/stb_image.h"

struct FloorVertex {
  glm::vec3 positions;
  glm::vec3 color;
  glm::vec2 texture_coords;
};

// Two triangles forming a 10×10 plane on the XZ axis
static const FloorVertex floor_vertices[] = {
    {{-5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}, {0.0f, 0.0f}},
    {{5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}, {1.0f, 0.0f}},
    {{5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}, {1.0f, 1.0f}},

    {{-5.f, 0.f, -5.f}, {0.3f, 0.3f, 0.3f}, {0.0f, 0.0f}},
    {{5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}, {1.0f, 1.0f}},
    {{-5.f, 0.f, 5.f}, {0.3f, 0.3f, 0.3f}, {0.0f, 1.0f}}};

static const char *vertex_shader_src = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 MVP;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	gl_Position = MVP * vec4(aPos, 1.0);
	ourColor = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)GLSL";

static const char *fragment_shader_src = R"GLSL(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);
}
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
    vao.AddVertexAttribute(2, GL_FLOAT, false); // texture coords
    vao.Apply();
    floor_vao_ = vao.GetVbo();

    // load and create a texture
    // -------------------------
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D,
                  texture); // all upcoming GL_TEXTURE_2D operations now have
                            // effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT); // set texture wrapping to GL_REPEAT (default
                                // wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can
    // find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data =
        stbi_load("data/textures/dirt.jpg", &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
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

    // bind Texture
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(program_);
    glUniformMatrix4fv(mvp_loc_, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(floor_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  void Tock(const struct FrameTick &tick, GlfwWindow &window) {}

  void DebugUI(GlfwWindow &window) { camera_.DebugUI(); }

private:
  unsigned int texture;

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
