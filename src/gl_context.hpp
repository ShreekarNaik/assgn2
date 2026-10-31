#pragma once

#include <functional>
#include <string>
#include <utility>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GLContext {
public:
  GLContext(int width, int height, const char *title);
  ~GLContext();

  GLContext(const GLContext &) = delete;
  GLContext &operator=(const GLContext &) = delete;

  bool should_close() const;
  void close();
  double time() const;
  std::pair<int, int> framebuffer_size() const;
  void swap_and_poll();
  void update_title(const std::string &suffix);

  using KeyCallback = std::function<void(int key, int action, bool shift)>;
  void on_key(KeyCallback cb);

private:
  static void glfw_key_callback(GLFWwindow *window, int key, int scancode,
                                int action, int mods);

  GLFWwindow *window_ = nullptr;
  std::string base_title_;
  KeyCallback key_cb_;
};
