#include "guihck.h"
#include <assert.h>
#include <stdio.h>

void callback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) listenedId;
  (void) property;
  (void) data;

  int next = scm_to_int8(value) + 1;
  printf("Setting element %d bar to %d\n", (int) listenerId, next);
  guihckElementProperty(ctx, listenerId, "bar", scm_from_int8(next));
}

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  guihckElementTypeFunctionMap fooMap = {NULL, NULL, NULL, NULL, NULL, NULL};

  guihckInit();
  guihckContext* ctx = guihckContextNew();
  guihckElementTypeId fooId = guihckElementTypeAdd(ctx, "foo", fooMap, 0);

  guihckElementId id1 = guihckElementNew(ctx, fooId, guihckContextGetRootElement(ctx));
  guihckElementId id2 = guihckElementNew(ctx, fooId, guihckContextGetRootElement(ctx));
  guihckElementId id3 = guihckElementNew(ctx, fooId, guihckContextGetRootElement(ctx));
  guihckElementId id4 = guihckElementNew(ctx, fooId, guihckContextGetRootElement(ctx));

  guihckElementProperty(ctx, id1, "bar", scm_from_int8(0));
  guihckElementProperty(ctx, id2, "bar", scm_from_int8(0));
  guihckElementProperty(ctx, id3, "bar", scm_from_int8(0));
  guihckElementProperty(ctx, id4, "bar", scm_from_int8(0));

  guihckElementAddListener(ctx, id2, id1, "bar", callback, NULL, NULL);
  guihckElementAddListener(ctx, id3, id2, "bar", callback, NULL, NULL);
  guihckElementAddListener(ctx, id4, id3, "bar", callback, NULL, NULL);

  guihckElementProperty(ctx,  id1, "bar", scm_from_int8(1));

  assert(scm_to_int8(guihckElementGetProperty(ctx, id1, "bar")) == 1);
  assert(scm_to_int8(guihckElementGetProperty(ctx, id2, "bar")) == 2);
  assert(scm_to_int8(guihckElementGetProperty(ctx, id3, "bar")) == 3);
  assert(scm_to_int8(guihckElementGetProperty(ctx, id4, "bar")) == 4);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}
