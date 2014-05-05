#include "guihck.h"

#include <stdio.h>

void initFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  printf("Initialize foo %d\n", id);
}
void destroyFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  printf("Destroy foo %d\n", id);
}
bool updateFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  printf("Updating foo %d\n", id);
  return true;
}

void renderFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  printf("Rendering foo %d\n", id);
}

int main(int argc, char** argv)
{
  guihckElementTypeFunctionMap fooMap = {initFoo, destroyFoo, updateFoo, renderFoo};

  // Trivial test for context
  guihckContext* ctx = guihckContextNew();
  guihckElementTypeId fooId = guihckElementTypeAdd(ctx, "foo", fooMap, 0);
  guihckElementId id = guihckElementNew(ctx, fooId, GUIHCK_NO_PARENT);
  guihckContextUpdate(ctx);
  guihckContextRender(ctx);
  guihckContextFree(ctx);

  // Trivial test for gui
  guihckGui* gui = guihckGuiNew();
  guihckElementTypeAdd(guihckGuiGetContext(gui), "foo", fooMap, 0);
  guihckGuiCreateElement(gui, "foo");
  guihckGuiPopElement(gui);
  guihckGuiUpdate(gui);
  guihckGuiRender(gui);
  guihckGuiFree(gui);

  return EXIT_SUCCESS;
}
