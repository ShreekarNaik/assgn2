#pragma once
#include "animation.hpp"
#include "minifig.hpp"
#include <memory>
class Scene {
public:
  Scene();
  ~Scene();
  void handle_key(int key, int action, bool shift);

  void render(GLuint program, float aspect) const;
  Keyframe state() const;
  void apply(const Keyframe &k);

private:
  Node *add_object(const geometry::Mesh &mesh, int texture = -1);
  void add_box(glm::vec3 center, glm::vec3 size, glm::vec3 color,
               int texture = -1);
  void build_room();
  void build_furniture();
  void build_lights_and_props();
  std::vector<std::unique_ptr<Minifig>> figures_;
  std::vector<std::unique_ptr<Node>> objects_;
  std::unique_ptr<Node> sky_, lantern_, lantern_glass_, torch_, torch_lens_,
      cup_;
  std::vector<Node *> lamp_faces_;
  std::vector<GLuint> textures_;
  int active_ = 0, camera_ = 0;
  int cup_holder_ = 0; // 0 counter, 1 chef right hand, 2 officer left hand
  float cup_tilt_ = 0;
  bool body_camera_ = false;
  std::array<bool, 2> walking_{{false, false}};

  std::array<int, 4> lights_{{1, 1, 0, 0}};
  glm::vec3 eye_{0, 7.9, 6.4}, target_{0, -.3, -.5};
};
