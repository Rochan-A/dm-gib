#include "engine/core/gl_core.h"
#include "engine/core/gl_window.h"

#include "third_party/concise_args/ConciseArgs.h"

#include "util/report/report.h"

#define MAX_BONE_INFLUENCE 4

int main(int argc, char **argv) {
  // Required to load in opengl.
  std::shared_ptr<gib::GLCore> core = std::make_shared<gib::GLCore>();
  std::string window_title{"Assimp test"};
  gib::GlfwWindow window(core, window_title);

  std::string model_path = "data/models/cobblestone/model.obj";
  assimp_util::Model model(model_path);

  return 1;
}