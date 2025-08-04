#pragma once

#include "engine/core/types.h"

#include <glm/glm.hpp>

namespace gib {

// Material Params. Ref:
// https://google.github.io/filament/main/materials.html#lit-model
struct MaterialParams {
  // Diffuse albedo for non-metallic surfaces, and specular color for metallic
  // surfaces
  BoundedType<glm::vec4> base_color{
      {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {1.f, 1.f, 1.f, 1.f}};
  // Perceived smoothness (1.0) or roughness (0.0) of a surface. Smooth surfaces
  // exhibit sharp reflections
  BoundedType<float> roughness{0.f, 0.f, 1.f};
  // Whether a surface appears to be dielectric (0.0) or conductor (1.0). Often
  // used as a binary value (0 or 1)
  BoundedType<float> metallic{0.f, 0.f, 1.f};
  // Fresnel reflectance at normal incidence for dielectric surfaces. This
  // directly controls the strength of the reflections
  BoundedType<float> reflectance{0.f, 0.f, 1.f};
  // Defines how much of the ambient light is accessible to a surface point. It
  // is a per-pixel shadowing factor between 0.0 and 1.0
  BoundedType<float> ambient_occlusion{0.f, 0.f, 1.f};
  // Amount of anisotropy in either the tangent or bitangent direction
  BoundedType<float> anisotropy{0.f, -1.f, 1.f};
  // Local surface direction in tangent space
  BoundedType<glm::vec3> anisotropy_direction{
      {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
  // Additional diffuse albedo to simulate emissive surfaces (such as neons,
  // etc.) This property is mostly useful in an HDR pipeline with a bloom pass.
  // Linear RGB intensity in nits, alpha encodes the exposure weight
  BoundedType<glm::vec4> emissive{
      {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {1.f, 1.f, 1.f, 1.f}};
  // A detail normal used to perturb the surface using bump mapping (normal
  // mapping)
  BoundedType<glm::vec3> normal{
      {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
  // Absorption factor for refractive objects
  BoundedType<glm::vec3> absorption{
      {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
  // Defines how much of the diffuse light of a dielectric is transmitted
  // through the object, in other words this defines how transparent an object
  // is
  BoundedType<float> transmission{0.f, 0.f, 1.f};
  // Strength factor between 0 and 1 for all shadows received by this material
  BoundedType<float> shadow_strength{0.f, 0.f, 1.f};
};

} // namespace gib