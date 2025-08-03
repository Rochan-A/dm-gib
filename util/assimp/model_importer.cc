#include "util/assimp/model_importer.h"
#include "engine/textures/texture.h"
#include "util/report/report.h"

namespace assimp_util {

Model::Model(const std::string &path, bool lazy_load) : path_(path) {
  if (!lazy_load) {
    LoadModelInternal(path);
  }
}

void Model::LoadModel() {
  if (textures_loaded_.empty() && meshes_.empty()) {
    LoadModelInternal(path_);
  }
}

void Model::LoadModelInternal(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  if ((scene == nullptr) ||
      ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0u) ||
      (scene->mRootNode == nullptr)) {
    THROW_FATAL("Failed to load ASSIMP model at path: {}.\nError: {}", path,
                importer.GetErrorString());
  }
  directory_ = path.substr(0, path.find_last_of('/'));

  ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode *node, const aiScene *scene) {
  for (unsigned int mesh_idx = 0; mesh_idx < node->mNumMeshes; ++mesh_idx) {
    // Node object only contains indices to index the actual objects in the
    // scene. Scene contains all the data, node is just to keep stuff organized
    // (like relations between nodes).
    aiMesh *mesh = scene->mMeshes[node->mMeshes[mesh_idx]];
    meshes_.push_back(ProcessMesh(mesh, scene));
  }
  // Recursively process each of the children nodes.
  for (unsigned int child_idx = 0; child_idx < node->mNumChildren;
       ++child_idx) {
    ProcessNode(node->mChildren[child_idx], scene);
  }
}

gib::Mesh Model::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
  // data to fill
  std::vector<gib::Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<gib::Texture> textures;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    gib::Vertex vertex;
    glm::vec3 vector;
    // we declare a placeholder vector since assimp uses its
    // own vector class that doesn't directly convert to
    // glm's vec3 class so we transfer the data to this
    // placeholder glm::vec3 first.

    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.position = vector;

    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.normal = vector;
    }

    if (mesh->mTextureCoords[0] != nullptr) {
      glm::vec2 vec;

      // Vertex can contain up to 8 different texture coordinates. We thus
      // make the assumption that we won't use models where a vertex can have
      // multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.texture_coords = vec;

      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.tangent = vector;

      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.bitangent = vector;
    } else {
      vertex.texture_coords = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (unsigned int mesh_face_idx = 0; mesh_face_idx < mesh->mNumFaces;
       ++mesh_face_idx) {
    const aiFace face = mesh->mFaces[mesh_face_idx];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse
  // texture should be named as 'texture_diffuseN' where N is a sequential
  // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other
  // texture as the following list summarizes: diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // Diffuse maps
  std::vector<gib::Texture> diffuse_maps =
      LoadMaterialTextures(material, aiTextureType_DIFFUSE);
  textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

  // Specular maps
  std::vector<gib::Texture> specular_maps =
      LoadMaterialTextures(material, aiTextureType_SPECULAR);
  textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

  // Normal maps
  std::vector<gib::Texture> normal_maps =
      LoadMaterialTextures(material, aiTextureType_HEIGHT);
  textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

  // Height maps
  std::vector<gib::Texture> height_maps =
      LoadMaterialTextures(material, aiTextureType_AMBIENT);
  textures.insert(textures.end(), height_maps.begin(), height_maps.end());

  return gib::Mesh(vertices, indices, textures);
}

std::vector<gib::Texture>
Model::LoadMaterialTextures(aiMaterial *material,
                            aiTextureType ai_texture_type) {
  std::vector<gib::Texture> textures;
  for (unsigned int texture_count = 0;
       texture_count < material->GetTextureCount(ai_texture_type);
       ++texture_count) {
    aiString texture_path;
    material->GetTexture(ai_texture_type, texture_count, &texture_path);

    const gib::TextureParams params{};
    const gib::Texture texture = gib::Texture::Load2D(
        fmt::format("{}/{}", directory_, texture_path.C_Str()), params);
    textures.push_back(texture);
    textures_loaded_.push_back(texture);
  }
  return textures;
}

} // namespace assimp_util