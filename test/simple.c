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

  guihckInit();
  // Trivial test for context
  guihckContext* ctx = guihckContextNew();
  guihckElementTypeId fooId = guihckElementTypeAdd(ctx, "foo", fooMap, 0);
  guihckElementId id = guihckElementNew(ctx, fooId, guihckContextGetRootElement(ctx));
  guihckContextUpdate(ctx);
  guihckContextRender(ctx);
  guihckContextFree(ctx);

  // Trivial test for stack operations
  guihckContext* ctx2 = guihckContextNew();
  guihckElementTypeAdd(ctx2, "foo", fooMap, 0);
  guihckStackPushNewElement(ctx2, "foo");
  guihckStackPopElement(ctx2);
  guihckContextUpdate(ctx2);
  guihckContextRender(ctx2);
  guihckContextFree(ctx2);

  return EXIT_SUCCESS;
}
