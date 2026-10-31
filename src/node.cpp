#include "node.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr GLuint kPositionAttribute = 0;
constexpr GLuint kColorAttribute = 1;

Node::Node(const geometry::Mesh &mesh, const glm::vec3 &joint_offset)
    : joint_offset_(joint_offset),
      vertex_count_(static_cast<GLsizei>(mesh.positions.size())) {
  if (vertex_count_ == 0)
    return;

  const GLsizeiptr position_bytes = mesh.positions.size() * sizeof(glm::vec4);
  const GLsizeiptr color_bytes = mesh.colors.size() * sizeof(glm::vec4);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               position_bytes + color_bytes +
                   mesh.normals.size() * sizeof(glm::vec3) +
                   mesh.uvs.size() * sizeof(glm::vec2),
               nullptr, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, position_bytes, mesh.positions.data());
  glBufferSubData(GL_ARRAY_BUFFER, position_bytes, color_bytes,
                  mesh.colors.data());

  glEnableVertexAttribArray(kPositionAttribute);
  glVertexAttribPointer(kPositionAttribute, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(kColorAttribute);
  glVertexAttribPointer(kColorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                        reinterpret_cast<const void *>(position_bytes));

  auto offset = position_bytes + color_bytes;
  glBufferSubData(GL_ARRAY_BUFFER, offset,
                  mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data());
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0,
                        reinterpret_cast<void *>(offset));
  offset += mesh.normals.size() * sizeof(glm::vec3);
  glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.uvs.size() * sizeof(glm::vec2),
                  mesh.uvs.data());
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0,
                        reinterpret_cast<void *>(offset));
  glBindVertexArray(0);
}

Node::~Node() {
  if (vbo_)
    glDeleteBuffers(1, &vbo_);
  if (vao_)
    glDeleteVertexArrays(1, &vao_);
}

Node *Node::add_child(const geometry::Mesh &mesh,
                      const glm::vec3 &joint_offset) {
  children_.push_back(std::make_unique<Node>(mesh, joint_offset));
  children_.back()->parent_ = this;
  return children_.back().get();
}

glm::mat4 Node::local_transform() const {
  glm::mat4 local =
      glm::translate(glm::mat4(1.0f), joint_offset_ + translation_);
  local = glm::rotate(local, glm::radians(rotation_.z), glm::vec3(0, 0, 1));
  local = glm::rotate(local, glm::radians(rotation_.y), glm::vec3(0, 1, 0));
  local = glm::rotate(local, glm::radians(rotation_.x), glm::vec3(1, 0, 0));

  return local;
}
glm::mat4 Node::world_transform() const {
  return parent_ ? parent_->world_transform() * local_transform()
                 : local_transform();
}
void Node::render(const glm::mat4 &parent_transform, GLint mvp_location) const {
  const glm::mat4 world = parent_transform * local_transform();

  if (vertex_count_ > 0) {
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glUniform1f(glGetUniformLocation(program, "emission"), emission_);
    glActiveTexture(GL_TEXTURE0);
    // Keep a complete texture bound even for untextured parts. The shader's
    // sampler still needs a valid binding on Apple's OpenGL driver.
    if (texture_ != 0)
      glBindTexture(GL_TEXTURE_2D, texture_);
    glUniform1i(glGetUniformLocation(program, "textured"), texture_ != 0);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(world));
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
  }

  for (const auto &child : children_) {
    child->render(world, mvp_location);
  }
}
