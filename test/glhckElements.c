#include "guihck.h"
#include "glhckElements.h"
#include "glhck/glhck.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

#define WIDTH 800
#define HEIGHT 480

char RUNNING = 1;

void windowCloseCallback(GLFWwindow* window)
{
  RUNNING = 0;
}

int main(int argc, char** argv)
{
  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "guihck-glhckElements", NULL, NULL);


  if(!window)
  {
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowCloseCallback(window, windowCloseCallback);

  if(!glhckContextCreate(argc, argv))
  {
    printf("GLHCK initialization error\n");
    return EXIT_FAILURE;
  }

  glhckLogColor(0);
  if(!glhckDisplayCreate(WIDTH, HEIGHT, GLHCK_RENDER_AUTO))
  {
    printf("GLHCK display create error");
    return EXIT_FAILURE;
  }

  glhckRenderClearColorb(128, 128, 128, 255);

  guihckGui* gui = guihckGuiNew();
  guihckGlhckAddAllTypes(guihckGuiGetContext(gui));

  guihckGuiCreateElement(gui, "rectangle");
  guihckGuiElementProperty(gui, "x", scm_from_double(200.0));
  guihckGuiElementProperty(gui, "y", scm_from_double(100.0));
  guihckGuiElementProperty(gui, "width", scm_from_double(50.0));
  guihckGuiElementProperty(gui, "height", scm_from_double(75.0));
  guihckGuiElementProperty(gui, "color", scm_list_3(scm_from_uint8(128), scm_from_uint8(52), scm_from_uint8(200)));
  guihckGuiPopElement(gui);

  while(RUNNING)
  {
    glfwPollEvents();
    guihckGuiUpdate(gui);

    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);
    guihckGuiRender(gui);
    glhckRender();
    glfwSwapBuffers(window);
  }

  guihckGuiFree(gui);

  return EXIT_SUCCESS;
}
