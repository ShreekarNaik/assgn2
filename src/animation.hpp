#pragma once
#include <array>
#include <string>
#include <vector>
// 4 lights, camera index, 6 camera coordinates, active figure,
// followed by each figure's 15 A1 degrees of freedom.
// Final fields: cup holder (0/1/2), cup tilt in degrees, officer body-camera
// mode, then chef/officer walk-cycle enable flags.
constexpr int Fields = 47;
constexpr int FPS = 30;
struct Keyframe {
  int frame = 0;
  std::array<float, Fields> v{};
};
std::vector<Keyframe> load_keyframes(const std::string &path);
void append_keyframe(const std::string &path, const Keyframe &k);
Keyframe sample(const std::vector<Keyframe> &keys, int frame);
