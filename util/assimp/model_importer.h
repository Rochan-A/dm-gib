#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "engine/core/gl_window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "util/report/report.h"

#include "engine/mesh/mesh.h"
#include "engine/textures/texture.h"
#include "engine/textures/texture_utils.h"

static constexpr int kMaxBoneInfulence = 4;

namespace assimp_util {

class Model {

public:
  explicit Model(const std::string &path, bool lazy_load = false);

  // Loads the model if not loaded.
  void LoadModel();

  [[nodiscard]] std::vector<gib::Texture> GetLoadedTexture() const {
    return textures_loaded_;
  }

  [[nodiscard]] std::vector<gib::Mesh> GetMeshes() const { return meshes_; }

private:
  gib::Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

  // Loads a model with supported ASSIMP extensions from file and stores the
  // resulting meshes in the meshes vector.
  void LoadModelInternal(const std::string &path);

  // Processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void ProcessNode(aiNode *node, const aiScene *scene);

  // Checks all material textures of a given type and loads the textures if
  // they're not loaded yet. the required info is returned as a TextureStruct
  // struct.
  std::vector<gib::Texture> LoadMaterialTextures(aiMaterial *material,
                                                 aiTextureType ai_texture_type);

  std::vector<gib::Texture> textures_loaded_;
  std::vector<gib::Mesh> meshes_;

  std::string path_;
  std::string directory_;
};

} // namespace assimp_util