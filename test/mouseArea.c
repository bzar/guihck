#include "guihck.h"
#include "glhckElements.h"
#include "guihckElements.h"
#include "glhck/glhck.h"
#include "GLFW/glfw3.h"
#include "glfwhck.h"

#include <stdio.h>

#define WIDTH 800
#define HEIGHT 480

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
  guihckElementsAddAllTypes(guihckGuiGetContext(gui));
  guihckGlhckAddAllTypes(guihckGuiGetContext(gui));
  guihckGuiExecuteScriptFile(gui, "scm/mouseArea.scm");

  glfwhckEventQueue* queue = glfwhckEventQueueNew(window, GLFWHCK_EVENTS_ALL);

  float mx = 0;
  float my = 0;
  char running = 1;
  while(running)
  {
    glfwPollEvents();

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    while(!glfwhckEventQueueEmpty(queue))
    {
      const glfwhckEvent* event = glfwhckEventQueuePop(queue);

      switch(event->type)
      {
        case GLFWHCK_EVENT_WINDOW_CLOSE:
        {
          running = 0;
          break;
        }
        case GLFWHCK_EVENT_MOUSE_BUTTON:
        {
          if(event->mouseButton.action == GLFW_PRESS)
          {
            guihckContextMouseDown(guihckGuiGetContext(gui), mx, my, event->mouseButton.button);
          }
          else if(event->mouseButton.action == GLFW_RELEASE)
          {
            guihckContextMouseUp(guihckGuiGetContext(gui), mx, my, event->mouseButton.button);
          }
          break;
        }
        case GLFWHCK_EVENT_MOUSE_POSITION:
        {
          guihckContextMouseMove(guihckGuiGetContext(gui), mx, my, event->mousePosition.x, event->mousePosition.y);
          mx = event->mousePosition.x;
          my = event->mousePosition.y;
          break;
        }
        default:
        {
          break;
        }
      }
    }

    guihckGuiUpdate(gui);

    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);
    guihckGuiRender(gui);
    glhckRender();
    glfwSwapBuffers(window);
  }

  glfwhckEventQueueFree(queue);
  guihckGuiFree(gui);

  return EXIT_SUCCESS;
}
