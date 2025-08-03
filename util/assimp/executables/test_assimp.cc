#include "engine/core/gl_window.h"

#include "util/assimp/model_importer.h"

#define MAX_BONE_INFLUENCE 4

int main() {
  // Required to load in opengl.
  std::string const window_title{"Assimp test"};
  gib::GlfwWindow window(window_title);

  std::string const model_path = "data/models/cobblestone/model.obj";
  assimp_util::Model model(model_path);

  return 1;
}