#include "gl_context.hpp"
#include "scene.hpp"
#include "shader_util.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
// GLFW supplies a monotonic clock, not a timer registration API. Dispatch a
// fixed-step application callback from its event loop; never sleep the UI
// thread.
class FrameTimer {
  double deadline_ = 0;

public:
  void reset() { deadline_ = glfwGetTime(); }
  bool dispatch(const std::function<void()> &callback) {
    if (glfwGetTime() < deadline_)
      return false;
    callback();
    deadline_ += 1.0 / FPS;
    return true;
  }
};
void capture(int index, int w, int h) {
  std::vector<unsigned char> pixels(w * h * 3);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
  std::ostringstream name;
  name << "frames/frame-" << std::setw(5) << std::setfill('0') << index
       << ".ppm";
  std::ofstream out(name.str(), std::ios::binary);
  out << "P6\n" << w << ' ' << h << "\n255\n";
  for (int y = h - 1; y >= 0; y--)
    out.write(reinterpret_cast<char *>(pixels.data() + y * w * 3), w * 3);
  if (!out)
    throw std::runtime_error("Frame write failed");
}
int main() try {
  GLContext window(960, 540, "CS675 A2 - One Last Cup");
  GLuint program =
      shader_util::load_program("shaders/basic.vert", "shaders/basic.frag");
  glUseProgram(program);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  Scene scene;
  std::vector<Keyframe> keys;
  FrameTimer timer;
  bool playing = false, recording = false;
  int frame = 0, index = 0;
  int record_w = 0, record_h = 0;
  auto load = [&]() {
    keys = load_keyframes("keyframes.txt");
    std::cout << "Loaded " << keys.size() << " keyframes\n";
  };
  auto start = [&](bool record) {
    if (keys.empty())
      load();
    if (record) {
      auto [w, h] = window.framebuffer_size();
      record_w = w;
      record_h = h;
      auto bytes =
          uintmax_t(w) * h * 3 * (keys.back().frame - keys.front().frame + 1);
      if (std::filesystem::space(".").available < bytes + 100000000)
        throw std::runtime_error("Insufficient disk space for PPM frames");
      if (std::filesystem::exists("frames") &&
          !std::filesystem::is_empty("frames")) {
        int take = 1;
        std::string backup;
        do {
          backup = "frames-previous-" + std::to_string(take++);
        } while (std::filesystem::exists(backup));
        std::filesystem::rename("frames", backup);
        std::cout << "Previous capture preserved in " << backup << '\n';
      }
      std::filesystem::create_directories("frames");
      std::cout << "Recording requires approximately " << bytes / 1000000
                << " MB\n";
    }
    frame = keys.front().frame;
    index = 0;
    playing = true;
    recording = record;
    timer.reset();
  };
  auto handle_key = [&](int key, int action, bool shift) {
    try {
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (playing) {
          playing = recording = false;
        } else
          window.close();
        return;
      }
      if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_L) {
          if (!playing)
            load();
          return;
        }
        if (key == GLFW_KEY_P || key == GLFW_KEY_R) {
          if (!playing)
            start(key == GLFW_KEY_R);
          return;
        }
        if (key == GLFW_KEY_S && !playing) {
          int stamp = 0;
          if (std::filesystem::exists("keyframes.txt") &&
              std::filesystem::file_size("keyframes.txt"))
            stamp = load_keyframes("keyframes.txt").back().frame + 10;
          auto k = scene.state();
          k.frame = stamp;
          append_keyframe("keyframes.txt", k);
          keys.clear();
          std::cout << "Saved frame " << stamp << '\n';
          return;
        }
      }
      if (!playing)
        scene.handle_key(key, action, shift);
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  };
  window.on_key(handle_key);
  while (!window.should_close()) {
    auto [w, h] = window.framebuffer_size();
    if (w <= 0 || h <= 0) {
      window.swap_and_poll();
      continue;
    }
    if (recording && (w != record_w || h != record_h))
      throw std::runtime_error(
          "Framebuffer resized during recording; keep dimensions fixed");
    bool tick = false;
    if (playing)
      tick = timer.dispatch([&] { scene.apply(sample(keys, frame)); });
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.render(program, float(w) / h);
    if (recording && tick)
      capture(index++, w, h);
    if (playing && tick) {
      if (frame == keys.back().frame) {
        playing = false;
        recording = false;
        std::cout << "Playback complete\n";
      } else
        ++frame;
    }
    window.swap_and_poll();
  }
  glDeleteProgram(program);
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
