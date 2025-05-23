//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#ifdef __APPLE__
// Defined before OpenGL and GLUT includes to avoid deprecation messages
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#endif

#include <iostream>

const size_t WIDTH = 640;
const size_t HEIGHT = 480;
const char *WINDOW_NAME = "Test OpenGL";

/*
 * Callback to handle the "close window" event, once the user pressed the Escape
 * key.
 */
static void quit_callback(GLFWwindow *window, int key, int scancode, int action,
                          int _mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
  GLFWwindow *window;

  if (!glfwInit()) {
    std::cerr << "ERROR: could not start GLFW3" << std::endl;
    return -1; // Initialize the lib
  }

  // Minimum target is OpenGL 4.1
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  window = glfwCreateWindow(HEIGHT, WIDTH, WINDOW_NAME, NULL, NULL);
  if (!window) {
    std::cerr << "ERROR: could not open window with GLFW3" << std::endl;
    glfwTerminate();
    return -1;
  }
  // Close the window as soon as the Escape key has been pressed
  glfwSetKeyCallback(window, quit_callback);
  // Makes the window context current
  glfwMakeContextCurrent(window);

  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  std::cout << "Renderer: " << renderer << std::endl;
  std::cout << "OpenGL version supported: " << version << std::endl;

  // Now we have a current OpenGL context, we can use OpenGL normally
  while (!glfwWindowShouldClose(window)) {
    // Render
    glClear(GL_COLOR_BUFFER_BIT);
    // Swap front and back buffers
    glfwSwapBuffers(window);
    // Poll for and process events
    glfwPollEvents();
  }

  // ... here, the user closed the window
  glfwTerminate();
  return 0;
}
