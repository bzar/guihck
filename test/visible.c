#include "guihck.h"

#include <stdio.h>
#include <assert.h>
void initFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  int* renderCount = data;
  *renderCount = 0;
}
void renderFoo(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  int* renderCount = data;
  *renderCount += 1;
}

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  guihckElementTypeFunctionMap fooMap = {initFoo, NULL, NULL, renderFoo, NULL, NULL};

  guihckInit();
  guihckContext* ctx = guihckContextNew();
  guihckElementTypeId foo1 = guihckElementTypeAdd(ctx, "foo", fooMap, sizeof(int));
  guihckElementNew(ctx, foo1, guihckContextGetRootElement(ctx));
  int* renderCount1 = guihckElementGetData(ctx, foo1);

  assert(*renderCount1 == 0);

  guihckContextRender(ctx);
  assert(*renderCount1 == 1);
  assert(guihckElementGetVisible(ctx, foo1));

  guihckElementVisible(ctx, foo1, false);
  assert(!guihckElementGetVisible(ctx, foo1));
  guihckContextRender(ctx);
  assert(*renderCount1 == 1);


  guihckElementVisible(ctx, foo1, true);
  guihckContextRender(ctx);
  assert(*renderCount1 == 2);

  guihckElementVisible(ctx, guihckContextGetRootElement(ctx), false);
  guihckContextRender(ctx);
  assert(*renderCount1 == 2);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}
