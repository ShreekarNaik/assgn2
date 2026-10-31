#include "scene.hpp"
#include "texture.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
using glm::vec3;
Scene::Scene() {
  for (auto name : {"chef", "police", "tiles", "wood", "sky"})
    textures_.push_back(load_texture(std::string("textures/") + name + ".ppm"));
  figures_.push_back(std::make_unique<Minifig>(chef_style()));
  figures_.push_back(std::make_unique<Minifig>(police_officer_style()));
  figures_[0]->texture(textures_[0]);
  figures_[1]->texture(textures_[1]);
  build_room();
  build_furniture();
  build_lights_and_props();
  sky_ = std::make_unique<Node>(
      geometry::box({0, 0, 0}, {160, 160, 160}, {1, 1, 1}), vec3(0));
  sky_->set_texture(textures_[4]);
  auto k = state();
  k.v[13] = 0;
  k.v[15] = -1.6;
  k.v[17] = -2;
  k.v[16] = -.1;
  k.v[28] = -25;
  k.v[30] = 3.4;
  k.v[32] = 1.8;
  k.v[31] = -.1;
  apply(k);
}
Node *Scene::add_object(const geometry::Mesh &mesh, int texture) {
  objects_.push_back(std::make_unique<Node>(mesh, vec3(0)));
  if (texture >= 0)
    objects_.back()->set_texture(textures_.at(texture));
  return objects_.back().get();
}
void Scene::add_box(vec3 center, vec3 size, vec3 color, int texture) {
  add_object(geometry::box(center, size, color), texture);
}
void Scene::build_room() {
  // Midnight diner: cream plaster, teal panelling and a walnut counter.
  add_box({0, -2.05, 0}, {18, .3, 14}, {.9, .86, .75}, 2);
  // Four walls and a ceiling. Rectangular panels leave real window/door holes.
  const vec3 plaster(.85, .79, .65), teal(.12, .3, .29), brass(.6, .46, .25);
  for (glm::vec4 panel :
       {glm::vec4(-9, 5.2, -1.9, .2), glm::vec4(7.8, 9, -1.9, 3.4),
        glm::vec4(-9, -7.75, .2, 3.4), glm::vec4(-3.45, 5.2, .2, 3.4),
        glm::vec4(5.2, 7.8, 3.1, 3.4), glm::vec4(-9, 9, 3.4, 8.3)}) {
    add_box({(panel.x + panel.y) / 2, (panel.z + panel.w) / 2, -6},
            {panel.y - panel.x, panel.w - panel.z, .3}, plaster);
  }
  for (float x : {-8.8f, 8.8f}) {
    add_box({x, 3.2, .5}, {.3, 10.2, 13}, plaster);
    add_box({x > 0 ? 8.6f : -8.6f, -.9, .5}, {.15, 2, 13}, teal);
  }
  add_box({0, 3.2, 7}, {18, 10.2, .3}, plaster);
  add_box({0, -.9, 6.8}, {18, 2, .15}, teal);
  add_box({0, 8.3, .5}, {18, .2, 13}, plaster);
  for (float x : {-6.f, 0.f, 6.f})
    add_box({x, 8.1, .5}, {.12, .2, 13}, {.32, .26, .18});
  // Low wall panels stop at the openings, rather than covering the sky view.
  add_box({-1.9, -.9, -5.8}, {14.2, 2, .15}, teal);
  add_box({8.4, -.9, -5.8}, {1.2, 2, .15}, teal);
  for (float x : {5.2f, 7.8f})
    add_box({x, .6, -5.8}, {.12, 5, .2}, brass);
  add_box({6.5, 3.1, -5.8}, {2.7, .12, .2}, brass);
  // Window frame and mullions surround a real opening onto the environment.
  for (float x : {-7.75f, -5.6f, -3.45f})
    add_box({x, 1.8, -5.58}, {.1, 3.3, .12}, {.6, .46, .25});
  for (float y : {.2f, 1.8f, 3.4f})
    add_box({-5.6, y, -5.58}, {4.4, .1, .12}, {.6, .46, .25});
}
void Scene::build_furniture() {
  // Back counter leaves an uninterrupted central walking lane.
  add_box({0, -.8, -3.75}, {9, 2, 1.2}, {.42, .28, .17}, 3);
  add_box({0, .25, -3.5}, {9.4, .2, 2}, {.88, .83, .68});
  add_box({0, -.2, -3.08}, {8.8, .12, .08}, {.68, .46, .2});
  for (float x : {-3.f, 0.f, 3.f}) {
    add_box({x, .42, -3.5}, {.8, .12, .7}, {.95, .91, .78});
    add_object(geometry::cylinder({x, .48, -3.5}, .25, .2, {.55, .24, .08}));
  }
  // Side seating stays outside the officer's approach path.
  for (float x : {-6.f, 6.f}) {
    add_box({x, -.3, 3.2}, {2.4, .18, 1.8}, {.62, .43, .24}, 3);
    add_box({x, -1.1, 3.2}, {.22, 1.5, .22}, {.17, .21, .2});
    for (float z : {1.8f, 4.6f}) {
      add_box({x, -1.3, z}, {1.1, .2, 1}, {.56, .2, .12});
      add_box({x, -1.65, z}, {.2, .6, .2}, {.18, .2, .18});
    }
    // A cup on each table gives a simple, readable sense of scale.
    add_object(geometry::cylinder({x, -.2, 3.2}, .18, .28, {.9, .87, .75}));
  }
  for (float y : {1.4f, 2.6f}) {
    add_box({1.6, y, -5.6}, {4.7, .15, .65}, {.48, .3, .17}, 3);
    for (float x : {-.2f, 1.f, 2.2f, 3.4f})
      add_object(
          geometry::cylinder({x, y + .08f, -5.55f}, .15, .42, {.27, .48, .43}));
  }
}
void Scene::build_lights_and_props() {
  // Pendant positions are also the two scene point-light positions.
  for (float x : {-6.f, 6.f}) {
    add_box({x, 5.65, .5}, {.055, 5.25, .055}, {.15, .17, .16});
    add_object(geometry::cylinder({x, 2.8, .5}, .55, .25, {.55, .36, .14}));
    add_object(geometry::cylinder({x, 2.78, .5}, .4, .035, {1, .84, .5}));
    lamp_faces_.push_back(objects_.back().get());
  }
  // The chef carries a hanging lantern; the officer carries a flashlight.
  auto brass = vec3(.5, .34, .13);
  auto lantern = geometry::cylinder({0, -.55, 0}, .23, .07, brass);
  lantern.append(geometry::cylinder({0, -.13, 0}, .25, .07, brass));
  for (float x : {-.16f, .16f})
    for (float z : {-.16f, .16f})
      lantern.append(geometry::box({x, -.32, z}, {.035, .42, .035}, brass));
  lantern.append(geometry::box({0, -.04, 0}, {.06, .2, .06}, brass));
  lantern_ = std::make_unique<Node>(lantern, vec3(0));
  lantern_glass_ = std::make_unique<Node>(
      geometry::cylinder({0, -.48, 0}, .14, .32, {1, .65, .24}), vec3(0));
  torch_ = std::make_unique<Node>(
      geometry::cylinder({0, -.1, 0}, .12, .48, {.12, .15, .16}), vec3(0));
  torch_lens_ = std::make_unique<Node>(
      geometry::cylinder({0, -.15, 0}, .16, .05, {.65, .82, 1}), vec3(0));
  // A single cup is either on the counter or carried by an existing wrist.
  // Put the handle on the viewer-facing side during pickup and transfer.
  auto mug = geometry::cylinder({0, 0, 0}, .19, .3, {.83, .3, .12});
  mug.append(geometry::cylinder({0, .302, 0}, .165, .008, {.11, .065, .03}));
  mug.append(geometry::box({-.36, .15, 0}, {.06, .23, .07}, {.83, .3, .12}));
  for (float y : {.04f, .26f})
    mug.append(geometry::box({-.27, y, 0}, {.24, .06, .07}, {.83, .3, .12}));
  cup_ = std::make_unique<Node>(mug, vec3(0));
}
Scene::~Scene() {
  for (auto id : textures_)
    glDeleteTextures(1, &id);
}
Keyframe Scene::state() const {
  Keyframe k;
  for (int i = 0; i < 4; i++)
    k.v[i] = lights_[i];
  k.v[4] = camera_;
  for (int i = 0; i < 3; i++) {
    k.v[5 + i] = eye_[i];
    k.v[8 + i] = target_[i];
  }
  k.v[11] = active_;
  k.v[42] = cup_holder_;
  k.v[43] = cup_tilt_;
  k.v[44] = body_camera_;
  k.v[45] = walking_[0];
  k.v[46] = walking_[1];
  for (int f = 0; f < 2; f++) {
    auto pose = figures_[f]->pose();
    for (int i = 0; i < 15; i++)
      k.v[12 + 15 * f + i] = pose[i];
  }
  return k;
}
void Scene::apply(const Keyframe &k) {
  for (int i = 0; i < 4; i++)
    lights_[i] = int(k.v[i]);
  camera_ = int(k.v[4]);
  for (int i = 0; i < 3; i++) {
    eye_[i] = k.v[5 + i];
    target_[i] = k.v[8 + i];
  }
  active_ = int(k.v[11]);
  cup_holder_ = int(k.v[42]);
  cup_tilt_ = k.v[43];
  body_camera_ = k.v[44] != 0;
  walking_[0] = k.v[45] != 0;
  walking_[1] = k.v[46] != 0;
  for (int f = 0; f < 2; f++)
    figures_[f]->set_pose(std::vector<float>(k.v.begin() + 12 + 15 * f,
                                             k.v.begin() + 27 + 15 * f));
}
void Scene::handle_key(int key, int action, bool shift) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_C) {
      camera_ = (camera_ + 1) % 3;
      return;
    }
    if (key == GLFW_KEY_TAB) {
      active_ = 1 - active_;
      return;
    }
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F4) {
      lights_[key - GLFW_KEY_F1] ^= 1;
      return;
    }
    if (key == GLFW_KEY_F) {
      body_camera_ = !body_camera_;
      return;
    }
    if (key == GLFW_KEY_M) {
      walking_[active_] = !walking_[active_];
      return;
    }
    if (key == GLFW_KEY_G) {
      cup_holder_ = (cup_holder_ + 1) % 3;
      return;
    }
    if (key == GLFW_KEY_BACKSPACE) {
      figures_[active_]->reset();
      return;
    }
  }
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    if (key == GLFW_KEY_A)
      cup_tilt_ = glm::clamp(cup_tilt_ + (shift ? -5.f : 5.f), -60.f, 60.f);
    float step = shift ? -.3f : .3f;
    for (int i = 0; i < 3; i++) {
      if (key == (i == 0 ? GLFW_KEY_J : i == 1 ? GLFW_KEY_K : GLFW_KEY_O))
        eye_[i] += step;
      if (key == GLFW_KEY_U + i)
        target_[i] += step;
    }
  }
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    figures_[active_]->handle_key(key, shift);
}
void Scene::render(GLuint program, float aspect) const {
  glUniform1i(glGetUniformLocation(program, "image"), 0);
  vec3 eye = eye_, target = target_, up(0, 1, 0);
  float fov = camera_ == 0 ? 74.f : 48.f;
  vec3 lamps[4] = {{-6, 2.78, .5}, {6, 2.78, .5}, {0, 0, 0}, {0, 0, 0}};
  auto chef_hand = figures_[0]->hand_transform(true);
  auto police_hand = figures_[1]->hand_transform(false);
  vec3 lantern_grip = vec3(chef_hand * glm::vec4(0, -.15, .23, 1));
  auto lantern_frame = glm::translate(glm::mat4(1), lantern_grip);
  auto torch_frame = glm::translate(police_hand, vec3(0, -.1, .23));
  lamps[2] = lantern_grip + vec3(0, -.32, 0);
  lamps[3] = vec3(torch_frame * glm::vec4(0, -.2, 0, 1));
  auto beam = glm::normalize(vec3(torch_frame * glm::vec4(0, -1, 0, 0)));
  glUniform3fv(glGetUniformLocation(program, "torch_direction"), 1,
               glm::value_ptr(beam));
  for (int f = 0; f < 2; f++) {
    auto p = figures_[f]->pose();
    if (camera_ == f + 1) {
      target = vec3(p[3], p[4] + (f == 0 ? 1.f : .65f), p[5]);
      eye = vec3(p[3] + (f == 0 ? 6.f : 1.f), p[4] + 2.8f,
                 p[5] + (f == 0 ? -1.5f : 4.5f));
      if (f == 1)
        fov = 65.f;
      if (f == 1 && body_camera_) {
        // Brief body-mounted view: actor stays steady, cafe moves behind him.
        auto body = figures_[f]->root_transform();
        eye = vec3(body * glm::vec4(.9, 1.45, 3.5, 1));
        target = vec3(body * glm::vec4(0, 1.45, 0, 1));
        up = vec3(body * glm::vec4(0, 1, 0, 0));
        fov = 65.f;
      }
    }
  }
  auto vp = glm::perspective(glm::radians(fov), aspect, .1f, 250.f) *
            glm::lookAt(eye, target, up);
  glUniformMatrix4fv(glGetUniformLocation(program, "vp"), 1, GL_FALSE,
                     glm::value_ptr(vp));
  glUniform3fv(glGetUniformLocation(program, "eye"), 1, glm::value_ptr(eye));
  glUniform3fv(glGetUniformLocation(program, "lights"), 4,
               glm::value_ptr(lamps[0]));
  glUniform1iv(glGetUniformLocation(program, "enabled"), 4, lights_.data());
  auto model = glGetUniformLocation(program, "uMVP");
  glUniform1i(glGetUniformLocation(program, "environment"), 1);
  sky_->render(glm::mat4(1), model);
  glUniform1i(glGetUniformLocation(program, "environment"), 0);
  for (int i = 0; i < 2; i++)
    lamp_faces_[i]->set_emission(lights_[i] ? .8f : 0.f);
  for (auto &o : objects_)
    o->render(glm::mat4(1), model);
  for (int f = 0; f < 2; f++) {
    figures_[f]->render(glm::mat4(1), model);
  }
  lantern_->render(lantern_frame, model);
  lantern_glass_->set_emission(lights_[2] ? 2.5f : 0.f);
  lantern_glass_->render(lantern_frame, model);
  torch_->render(torch_frame, model);
  torch_lens_->set_emission(lights_[3] ? 1.5f : 0.f);
  torch_lens_->render(torch_frame, model);
  vec3 cup_position(-.8, .35, -2.779);
  if (cup_holder_ != 0) {
    auto hand = figures_[cup_holder_ - 1]->hand_transform(cup_holder_ == 2);
    cup_position = vec3(hand * glm::vec4(0, -.08, .23, 1)) + vec3(0, -.1, 0);
  }
  auto cup_frame = glm::translate(glm::mat4(1), cup_position);
  if (cup_holder_ == 0)
    cup_frame = glm::rotate(cup_frame, glm::radians(90.f), vec3(0, 1, 0));
  if (cup_holder_ != 0) {
    // The receiver faces the giver. Its opposite grip needs a half-turn
    // offset, so changing holder does not flip the cup in world space.
    float grip_offset = cup_holder_ == 2 ? 90.f : -90.f;
    cup_frame = glm::rotate(
        cup_frame,
        glm::radians(figures_[cup_holder_ - 1]->pose()[1] + grip_offset),
        vec3(0, 1, 0));
  }
  cup_frame = glm::rotate(cup_frame, glm::radians(cup_tilt_), vec3(1, 0, 0));
  cup_->render(cup_frame, model);
}
