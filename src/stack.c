#include "internal.h"
#include <assert.h>

void guihckStackPushNewElement(guihckContext* ctx, const char* typeName)
{
  guihckElementId* parentId = chckIterPoolGetLast(ctx->stack);
  guihckElementTypeId* typeId = chckHashTableStrGet(ctx->elementTypesByName, typeName);
  assert(typeId && "Element type not found");

  guihckElementId id = guihckElementNew(ctx, *typeId, *parentId);
  chckIterPoolAdd(ctx->stack, &id, NULL);
}

void guihckStackPushElement(guihckContext* ctx, guihckElementId elementId)
{
  chckIterPoolAdd(ctx->stack, &elementId, NULL);
}

void guihckStackPushElementById(guihckContext* ctx, const char* idValue)
{
  SCM idstr = scm_string_to_symbol(scm_from_utf8_string(idValue));

  /* Start search from current */
  guihckElementId initialId = *(guihckElementId*) chckIterPoolGetLast(ctx->stack);
  if(scm_is_eq(guihckElementGetProperty(ctx, initialId, "id"), idstr))
  {
    /* Result found */
    guihckStackPushElement(ctx, initialId);
    return;
  }

  /* Search ancestors */
  guihckElementId ancestorId = initialId;
  while((ancestorId = guihckElementGetParent(ctx, ancestorId)) != GUIHCK_NO_PARENT)
  {
    if(scm_is_eq(guihckElementGetProperty(ctx, ancestorId, "id"), idstr))
    {
      /* Result found */
      guihckStackPushElement(ctx, ancestorId);
      return;
    }
  }

  /* Search children with BFS */
  chckRingPool* queue = chckRingPoolNew(16, 16, sizeof(guihckElementId));
  chckRingPoolPushEnd(queue, &initialId);

  guihckElementId* id;
  while((id = chckRingPoolPopFirst(queue)))
  {
    if(scm_is_eq(guihckElementGetProperty(ctx, *id, "id"), idstr))
    {
      /* Result found */
      guihckStackPushElement(ctx, *id);
      chckRingPoolFree(queue);
      return;
    }

    /* Add children to queue */
    unsigned int i;
    for(i = 0; i < guihckElementGetChildCount(ctx, *id); ++i)
    {
      guihckElementId childId = guihckElementGetChild(ctx, *id, i);
      chckRingPoolPushEnd(queue, &childId);
    }
  }
  chckRingPoolFree(queue);

  /* Search sibling */
  guihckElementId parentId = guihckElementGetParent(ctx, initialId);
  if(parentId != GUIHCK_NO_PARENT)
  {
    int siblings = guihckElementGetChildCount(ctx, parentId);
    int i;
    for(i = 0; i < siblings; ++i)
    {
      guihckElementId siblingId = guihckElementGetChild(ctx, parentId, i);

      if(siblingId == initialId)
        continue;

      if(scm_is_eq(guihckElementGetProperty(ctx, siblingId, "id"), idstr))
      {
        /* Result found */
        guihckStackPushElement(ctx, siblingId);
        return;
      }
    }
  }

  assert(false && "Could not find element with requested id");
}

void guihckStackPushParentElement(guihckContext* ctx)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  guihckElementId parentId = guihckElementGetParent(ctx, *id);
  guihckStackPushElement(ctx, parentId);
}

void guihckStackPushChildElement(guihckContext* ctx, int childIndex)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  guihckElementId childId = guihckElementGetChild(ctx, *id, childIndex);
  guihckStackPushElement(ctx, childId);
}

void guihckStackPopElement(guihckContext* ctx)
{
  size_t size = chckIterPoolCount(ctx->stack);
  assert(size > 1 && "Tried to pop root element from element stack");
  chckIterPoolRemove(ctx->stack, size - 1);
}

guihckElementId guihckStackGetElement(guihckContext* ctx)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  return *id;
}

SCM guihckStackGetElementProperty(guihckContext* ctx, const char* key)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  return guihckElementGetProperty(ctx, *id,  key);
}


void guihckStackElementProperty(guihckContext* ctx, const char* key, SCM value)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  return guihckElementProperty(ctx, *id,  key, value);
}

int guihckStackGetElementChildCount(guihckContext* ctx)
{
  guihckElementId* id = chckIterPoolGetLast(ctx->stack);
  return guihckElementGetChildCount(ctx, *id);
}
