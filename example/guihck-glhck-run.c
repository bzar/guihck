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
  if(argc != 2)
  {
    printf("Usage: guihck-glhck-run <scm file>\n");
    return EXIT_FAILURE;
  }

  if (!glfwInit())
    return EXIT_FAILURE;

  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "guihck-glhck-run", NULL, NULL);


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

  guihckInit();

  guihckContext* ctx = guihckContextNew();
  guihckElementsAddAllTypes(ctx);
  guihckGlhckAddAllTypes(ctx);
  guihckContextExecuteScriptFile(ctx, argv[1]);

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
            guihckContextMouseDown(ctx, mx, my, event->mouseButton.button);
          }
          else if(event->mouseButton.action == GLFW_RELEASE)
          {
            guihckContextMouseUp(ctx, mx, my, event->mouseButton.button);
          }
          break;
        }
        case GLFWHCK_EVENT_KEYBOARD_CHAR:
        {
          guihckContextKeyboardChar(ctx, event->keyboardChar.codepoint);
        }
        case GLFWHCK_EVENT_KEYBOARD_KEY:
        {
          guihckContextKeyboardKey(ctx, event->keyboardKey.key, event->keyboardKey.scancode, event->keyboardKey.action, event->keyboardKey.mods);
        }
        case GLFWHCK_EVENT_MOUSE_POSITION:
        {
          guihckContextMouseMove(ctx, mx, my, event->mousePosition.x, event->mousePosition.y);
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

    guihckContextUpdate(ctx);

    glhckRenderClear(GLHCK_DEPTH_BUFFER_BIT | GLHCK_COLOR_BUFFER_BIT);
    guihckContextRender(ctx);
    glhckRender();
    glfwSwapBuffers(window);
  }

  glfwhckEventQueueFree(queue);
  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}
