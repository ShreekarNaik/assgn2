#include "gl_context.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

GLContext::GLContext(int width, int height, const char *title)
    : base_title_(title) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialise GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    glfwDestroyWindow(window_);
    glfwTerminate();
    throw std::runtime_error("Failed to initialise GLEW");
  }
  glGetError(); // Clear harmless error from glewInit

  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, glfw_key_callback);
}

GLContext::~GLContext() {
  if (window_) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}

void GLContext::close() { glfwSetWindowShouldClose(window_, GL_TRUE); }
bool GLContext::should_close() const { return glfwWindowShouldClose(window_); }

double GLContext::time() const { return glfwGetTime(); }

std::pair<int, int> GLContext::framebuffer_size() const {
  int w = 0, h = 0;
  glfwGetFramebufferSize(window_, &w, &h);
  return {w, h};
}

void GLContext::swap_and_poll() {
  glfwSwapBuffers(window_);
  glfwPollEvents();
}

void GLContext::update_title(const std::string &suffix) {
  std::string full_title = base_title_;
  if (!suffix.empty()) {
    full_title += " - " + suffix;
  }
  glfwSetWindowTitle(window_, full_title.c_str());
}

void GLContext::on_key(KeyCallback cb) { key_cb_ = std::move(cb); }

void GLContext::glfw_key_callback(GLFWwindow *window, int key, int /*scancode*/,
                                  int action, int mods) {
  if (auto *ctx = static_cast<GLContext *>(glfwGetWindowUserPointer(window))) {
    if (ctx->key_cb_) {
      ctx->key_cb_(key, action, (mods & GLFW_MOD_SHIFT) != 0);
    }
  }
}
