#pragma once

#include <memory>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "geometry.hpp"

class Node {
public:
  Node(const geometry::Mesh &mesh, const glm::vec3 &joint_offset);
  ~Node();

  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

  Node *add_child(const geometry::Mesh &mesh, const glm::vec3 &joint_offset);

  void render(const glm::mat4 &parent_transform, GLint mvp_location) const;

  glm::mat4 local_transform() const;
  glm::mat4 world_transform() const;

  void set_rotation(int axis, float degrees) { rotation_[axis] = degrees; }
  void set_translation(int axis, float value) { translation_[axis] = value; }

  void set_emission(float value) { emission_ = value; }
  void set_texture(GLuint texture) { texture_ = texture; }

private:
  Node *parent_ = nullptr;
  GLuint texture_ = 0;
  float emission_ = 0;
  glm::vec3 joint_offset_;
  glm::vec3 rotation_{0.0f};
  glm::vec3 translation_{0.0f};

  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  GLsizei vertex_count_ = 0;

  std::vector<std::unique_ptr<Node>> children_;
};
