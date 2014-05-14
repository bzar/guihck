#include "guihck.h"
#include "guihckGuile.h"
#include "pool.h"
#include "lut.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define GUIHCK_NO_PARENT -1

// temporary
typedef struct _guihckKeyValueMapping
{
  char* key;
  SCM value;
} _guihckKeyValueMapping;

typedef struct _guihckContext
{
  chckPool* elements;
  chckPool* elementTypes;
  chckHashTable* elementTypesByName;
  chckPool* mouseAreas;  /* should also have a quadtree for references */
  chckIterPool* stack;
  guihckElementId rootElementId;
  chckPool* propertyListeners;
} _guihckContext;

typedef struct _guihckElementType
{
  char* name;
  guihckElementTypeFunctionMap functionMap;
  size_t dataSize;
} _guihckElementType;

typedef struct _guihckRect
{
    float x;
    float y;
    float w;
    float h;

} _guihckRect;

typedef struct _guihckPropertyListener
{
  guihckElementId listenerId;
  guihckElementId listenedId;
  char* propertyName;
  guihckPropertyListenerCallback callback;
  void* data;
} _guihckPropertyListener;

typedef struct _guihckProperty
{
  SCM value;
  chckIterPool* listeners;
} _guihckProperty;

typedef struct _guihckElement
{
  guihckElementTypeId type;
  void* data;
  guihckElementId parent;
  chckIterPool* children;
  chckHashTable* properties;
  chckIterPool* listened;
  bool dirty;
} _guihckElement;

typedef struct _guihckMouseArea
{
  guihckElementId elementId;
  _guihckRect rect;
  guihckMouseAreaFunctionMap functionMap;
} _guihckMouseArea;

static char pointInRect(float x, float y, const _guihckRect* r);

void guihckInit()
{
  guihckGuileInit();
}

void guihckRegisterFunction(const char* name, int req, int opt, int rst, scm_t_subr func)
{
  guihckGuileRegisterFunction(name, req, opt, rst, func);
}

guihckContext* guihckContextNew()
{
  guihckContext* ctx = calloc(1, sizeof(guihckContext));
  ctx->elements = chckPoolNew(64, 64, sizeof(guihckElement));
  ctx->elementTypes = chckPoolNew(16, 16, sizeof(_guihckElementType));
  ctx->elementTypesByName = chckHashTableNew(32);
  ctx->mouseAreas = chckPoolNew(16, 16, sizeof(_guihckMouseArea));
  ctx->stack = chckIterPoolNew(16, 16, sizeof(guihckElementId));
  ctx->propertyListeners = chckPoolNew(16, 16, sizeof(_guihckPropertyListener));

  guihckElementTypeFunctionMap rootElementFunctionMap = { NULL, NULL, NULL, NULL };
  guihckElementTypeId rootTypeId = guihckElementTypeAdd(ctx, "root", rootElementFunctionMap, 0);
  ctx->rootElementId = guihckElementNew(ctx, rootTypeId, GUIHCK_NO_PARENT);
  guihckElementProperty(ctx, ctx->rootElementId, "id", scm_from_utf8_string("root"));
  guihckStackPushElement(ctx, ctx->rootElementId);
  return ctx;
}


void guihckContextFree(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
    if(type->functionMap.destroy)
      type->functionMap.destroy(ctx, iter - 1, current->data); /* id = iter - 1 */

    _guihckProperty* property;
    chckHashTableIterator pIter = {NULL, 0};

    if(current->listened)
      chckIterPoolFree(current->listened);

    while(property = chckHashTableIter(current->properties, &pIter))
    {
      if(property->listeners)
        chckIterPoolFree(property->listeners);
    }

    chckHashTableFree(current->properties);
    chckIterPoolFree(current->children);
    free(current->data);
  }

  _guihckPropertyListener* listener;
  iter = 0;
  while(listener = chckPoolIter(ctx->propertyListeners, &iter))
  {
    free(listener->propertyName);
    if(listener->data)
      free(listener->data);
  }
  chckPoolFree(ctx->propertyListeners);

  chckPoolFree(ctx->mouseAreas);
  chckPoolFree(ctx->elements);
  chckHashTableFree(ctx->elementTypesByName);
  chckPoolFree(ctx->elementTypes);
  free(ctx);
}

void guihckContextUpdate(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    if(current->dirty)
    {
      _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
      assert(type && "Invalid element type");
      current->dirty = false;
      if(type->functionMap.update)
      {
        if(type->functionMap.update(ctx, iter - 1, current->data)) /* id = iterator - 1 */
        {
          guihckElementDirty(ctx, iter - 1);
        }
      }
    }
  }
}


void guihckContextRender(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
    if(type->functionMap.render)
      type->functionMap.render(ctx, iter - 1, current->data); /* id = iterator - 1 */
  }
}

guihckElementId guihckContextGetRootElement(guihckContext* ctx)
{
  return ctx->rootElementId;
}

guihckElementTypeId guihckElementTypeAdd(guihckContext* ctx, const char* name, guihckElementTypeFunctionMap functionMap, size_t dataSize)
{
  chckPoolIndex iter = 0;
  _guihckElementType* current;
  while ((current = chckPoolIter(ctx->elementTypes, &iter)))
  {
    assert(strcmp(current->name, name) != 0 && "Element type with this name already exists");
  }

  _guihckElementType type;
  type.name = strdup(name);
  type.functionMap = functionMap;
  type.dataSize = dataSize;

  guihckElementTypeId id = -1;
  chckPoolAdd(ctx->elementTypes, &type, &id);
  chckHashTableStrSet(ctx->elementTypesByName, name, &id, sizeof(guihckElementTypeId));

  return id;
}

guihckElementId guihckElementNew(guihckContext* ctx, guihckElementTypeId typeId, guihckElementId parentId)
{
  _guihckElementType* type = chckPoolGet(ctx->elementTypes, typeId);
  guihckElement element;
  element.type = typeId;
  element.data = type->dataSize > 0 ? calloc(1, type->dataSize) : NULL;
  element.parent = parentId;
  element.children = chckIterPoolNew(8, 8, sizeof(guihckElementId));
  element.properties = chckHashTableNew(32);
  element.listened = NULL;
  element.dirty = true;

  guihckElementId id = -1;
  chckPoolAdd(ctx->elements, &element, &id);


  guihckElement* parent = chckPoolGet(ctx->elements, parentId);
  if(parent)
  {
    chckIterPoolAdd(parent->children, &id, NULL);
    guihckElementDirty(ctx, parentId);
  }

  if(type->functionMap.init)
    type->functionMap.init(ctx, id, element.data);

  return id;
}

static void removeListeners(guihckContext* ctx, chckIterPool* pool)
{
  if(chckIterPoolCount(pool) > 0)
  {
    /* Copy listener ids before removing as modifying the pool may break iterators */
    size_t listenedCount;
    guihckPropertyListenerId* listenerIdsOrig = chckIterPoolToCArray(pool, &listenedCount);
    guihckPropertyListenerId* listenerIds = calloc(listenedCount, sizeof(guihckPropertyListenerId));
    memcpy(listenerIds, listenerIdsOrig, listenedCount * sizeof(guihckPropertyListenerId));

    int i;
    for(i = 0; i < listenedCount; ++i)
    {
      guihckElementRemoveListener(ctx, listenerIds[i]);
    }
    free(listenerIds);
  }
}

void guihckElementRemove(guihckContext* ctx, guihckElementId elementId)
{
  assert(elementId != ctx->rootElementId && "Tried to remove root element");
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckElementType* type = chckPoolGet(ctx->elementTypes, element->type);

  /* Execute destructor */
  if(type->functionMap.destroy)
    type->functionMap.destroy(ctx, elementId, element->data);

  /* Remove property listeners for listened */
  if(element->listened)
  {
    removeListeners(ctx, element->listened);
    chckIterPoolFree(element->listened);
  }

  /* Remove property listeners for listeners */
  _guihckProperty* property;
  chckHashTableIterator pIter = {NULL, 0};
  while(property = chckHashTableIter(element->properties, &pIter))
  {
    if(property->listeners)
    {
      removeListeners(ctx, property->listeners);
      chckIterPoolFree(property->listeners);
    }
  }

  /* Remove properties */
  chckHashTableFree(element->properties);

  /* Remove element data */
  if(element->data)
    free(element->data);

  /* Remove children */
  chckIterPool* children = element->children; /* element may be invalidated while removing children  */
  chckPoolIndex iter = 0;
  guihckElementId* current;
  while ((current = chckIterPoolIter(children, &iter)))
  {
    guihckElementRemove(ctx, *current);
  }
  chckIterPoolFree(children);

  chckPoolRemove(ctx->elements, elementId);
}


SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char* key)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckProperty* prop = chckHashTableStrGet(element->properties, key);

  if(!prop)
  {
    return SCM_UNDEFINED;
  }

  if(scm_is_pair(prop->value) && scm_is_symbol(SCM_CAR(prop->value)))
  {
    SCM first = SCM_CAR(prop->value);

    if(scm_is_eq(first, scm_string_to_symbol(scm_from_utf8_string("bind"))))
    {
      // Property is a bound value; determining value
      guihckStackPushElement(ctx, elementId);
      SCM binding = SCM_CADR(prop->value);
      prop->value = guihckContextExecuteExpression(ctx, binding);
      guihckStackPopElement(ctx);
    }
    else if(scm_is_eq(first, scm_string_to_symbol(scm_from_utf8_string("alias"))))
    {
      // Property is an alias
      guihckStackPushElement(ctx, elementId);
      SCM aliasedElementExpression = SCM_CADR(prop->value);
      SCM aliasedElement = guihckContextExecuteExpression(ctx, aliasedElementExpression);
      SCM aliasedKey = SCM_CADDR(prop->value);

      guihckStackPopElement(ctx);

      guihckElementId aliasedElementId = scm_to_int64(aliasedElement);
      char* aliasedKeyString = scm_to_utf8_string(scm_symbol_to_string(aliasedKey));
      prop->value = guihckElementGetProperty(ctx, aliasedElementId, aliasedKeyString);
      free(aliasedKeyString);
    }


  }

  return prop->value;
}

static bool _guihckPropertyIsAnAlias(SCM value)
{
  return scm_is_pair(value) && scm_is_symbol(SCM_CAR(value))
      && scm_is_eq(SCM_CAR(value), scm_string_to_symbol(scm_from_utf8_string("alias")));
}

static void _guihckElementAliasedProperty(guihckContext* ctx, guihckElementId elementId, SCM alias, SCM value)
{
  guihckStackPushElement(ctx, elementId);
  SCM aliasedElementExpression = SCM_CADR(alias);
  SCM aliasedElement = guihckContextExecuteExpression(ctx, aliasedElementExpression);
  SCM aliasedKey = SCM_CADDR(alias);

  guihckStackPopElement(ctx);

  guihckElementId aliasedElementId = scm_to_int64(aliasedElement);
  char* aliasedKeyString = scm_to_utf8_string(scm_symbol_to_string(aliasedKey));
  guihckElementProperty(ctx, aliasedElementId, aliasedKeyString, value);
  free(aliasedKeyString);
}

void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckProperty* prop = chckHashTableStrGet(element->properties, key);

  if(prop && scm_is_pair(prop->value) && _guihckPropertyIsAnAlias(prop->value))
  {
    // Property is an alias, set target instead
    _guihckElementAliasedProperty(ctx, elementId, prop->value, value);
  }
  else
  {
    bool newValue = !prop;

    if(prop && _guihckPropertyIsAnAlias(value))
    {
      // New value is an alias, set old value into target
      _guihckElementAliasedProperty(ctx, elementId, value, prop->value);
    }

    bool valueChanged = !newValue && !scm_is_eq(prop->value, value);

    if(newValue)
    {
      _guihckProperty newProp;
      newProp.value = value;
      newProp.listeners = NULL;
      chckHashTableStrSet(element->properties, key, &newProp, sizeof(_guihckProperty));
    }
    else if(valueChanged)
    {
      prop->value = value;

      if(prop->listeners)
      {
        chckPoolIndex iter = 0;
        guihckPropertyListenerId* listenerId;
        while(listenerId = chckIterPoolIter(prop->listeners, &iter))
        {
          _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, *listenerId);
          listener->callback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, value, listener->data);
        }
      }

      /* TODO: implement using listeners */
      size_t changeHandlerKeySize = snprintf(NULL, 0, "on-%s", key);
      char* changeHandlerKey = calloc(sizeof(char), changeHandlerKeySize);
      sprintf(changeHandlerKey, "on-%s", key);
      SCM changeHandler = guihckElementGetProperty(ctx, elementId, changeHandlerKey);
      free(changeHandlerKey);
      if(scm_is_pair(changeHandler))
      {
        guihckStackPushElement(ctx, elementId);
        guihckContextExecuteExpression(ctx, changeHandler);
        guihckStackPopElement(ctx);
      }
    }

    if(newValue || valueChanged)
    {
      guihckElementDirty(ctx, elementId);
    }
  }
}


guihckElementId guihckElementGetParent(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  return element->parent;
}

size_t guihckElementGetChildCount(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  return chckIterPoolCount(element->children) ;
}

guihckElementId guihckElementGetChild(guihckContext* ctx, guihckElementId elementId, int childIndex)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  chckPoolIndex iter = 0;
  guihckElementId* childId;
  int i = 0;
  while ((childId = chckIterPoolIter(element->children, &iter)) && i++ < childIndex);
  return *childId;
}

void guihckElementGetChildren(guihckContext* ctx, guihckElementId elementId, guihckElementId* children)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  chckIterPoolSetCArray(element->children, children, chckIterPoolCount(element->children));
}

void guihckElementDirty(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  element->dirty = true;

  chckPoolIndex iter = 0;
  guihckElementId* childId;
  while ((childId = chckIterPoolIter(element->children, &iter)))
    guihckElementDirty(ctx, *childId);
}

void* guihckElementGetData(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  return element->data;
}

guihckPropertyListenerId guihckElementAddListener(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId,
                                                  const char* propertyName, guihckPropertyListenerCallback callback, void* data)
{
  _guihckPropertyListener propertyListener;
  propertyListener.listenerId = listenerId;
  propertyListener.listenedId = listenedId;
  propertyListener.propertyName = strdup(propertyName);
  propertyListener.callback = callback;
  propertyListener.data = data;
  guihckPropertyListenerId id;
  chckPoolAdd(ctx->propertyListeners, &propertyListener, &id);

  guihckElement* listenedElement = chckPoolGet(ctx->elements, listenedId);
  _guihckProperty* property = chckHashTableStrGet(listenedElement->properties, propertyName);
  if(!property->listeners)
    property->listeners = chckIterPoolNew(4, 4, sizeof(guihckPropertyListenerId));
  chckIterPoolAdd(property->listeners, &id, NULL);

  guihckElement* listenerElement = chckPoolGet(ctx->elements, listenerId);
  if(!listenerElement->listened)
    listenerElement->listened = chckIterPoolNew(4, 4, sizeof(guihckPropertyListenerId));

  chckIterPoolAdd(listenerElement->listened, &id, NULL);
}

void guihckElementRemoveListener(guihckContext* ctx, guihckPropertyListenerId propertyListenerId)
{
  _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, propertyListenerId);

  if(!listener)
    return;

  guihckElement* listenedElement = chckPoolGet(ctx->elements, listener->listenedId);
  _guihckProperty* property = chckHashTableStrGet(listenedElement->properties, listener->propertyName);

  chckPoolIndex iter = 0;
  guihckPropertyListenerId* id;
  while((id = chckIterPoolIter(property->listeners, &iter)) && *id != propertyListenerId);

  if(id)
    chckIterPoolRemove(property->listeners, *id);

  guihckElement* listenerElement = chckPoolGet(ctx->elements, listener->listenerId);

  if(listenerElement)
  {
    iter = 0;
    id = NULL;
    while((id = chckIterPoolIter(listenerElement->listened, &iter)) && *id != propertyListenerId);

    if(id)
      chckIterPoolRemove(listenerElement->listened, *id);

    if(listener->data)
      free(listener->data);

    free(listener->propertyName);
  }
  chckPoolRemove(ctx->propertyListeners, propertyListenerId);
}


SCM guihckContextExecuteExpression(guihckContext* ctx, SCM expression)
{
  return guihckGuileRunExpression(ctx, expression);
}

SCM guihckContextExecuteScript(guihckContext* ctx, const char* script)
{
  return guihckGuileRunScript(ctx, script);
}

SCM guihckContextExecuteScriptFile(guihckContext* ctx, const char* path)
{
  FILE* file;

  if(file = fopen(path, "rb"))
  {
    /* Read file into memory */
    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* contents = calloc(pos, 1);
    fread(contents, pos, 1, file);
    fclose(file);

    guihckContextExecuteScript(ctx, contents);
    free(contents);
  }
}

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

  // Start search from current
  guihckElementId initialId = *(guihckElementId*) chckIterPoolGetLast(ctx->stack);
  if(scm_is_eq(guihckElementGetProperty(ctx, initialId, "id"), idstr))
  {
    // Result found
    guihckStackPushElement(ctx, initialId);
    return;
  }

  // Search ancestors
  guihckElementId ancestorId = initialId;
  while((ancestorId = guihckElementGetParent(ctx, ancestorId)) != GUIHCK_NO_PARENT)
  {
    if(scm_is_eq(guihckElementGetProperty(ctx, ancestorId, "id"), idstr))
    {
      // Result found
      guihckStackPushElement(ctx, ancestorId);
      return;
    }
  }

  // Search children with BFS
  chckRingPool* queue = chckRingPoolNew(16, 16, sizeof(guihckElementId));
  chckRingPoolPushEnd(queue, &initialId);

  guihckElementId* id;
  while(id = chckRingPoolPopFirst(queue))
  {
    if(scm_is_eq(guihckElementGetProperty(ctx, *id, "id"), idstr))
    {
      // Result found
      guihckStackPushElement(ctx, *id);
      chckRingPoolFree(queue);
      return;
    }

    // Add children to queue
    int i;
    for(i = 0; i < guihckElementGetChildCount(ctx, *id); ++i)
    {
      guihckElementId childId = guihckElementGetChild(ctx, *id, i);
      chckRingPoolPushEnd(queue, &childId);
    }
  }
  chckRingPoolFree(queue);

  // Search siblings
  guihckElementId parentId = guihckElementGetParent(ctx, initialId);
  if(ancestorId != GUIHCK_NO_PARENT);
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
        // Result found
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

void guihckContextMouseDown(guihckContext* ctx, float x, float y, int button)
{
  /* Should be replaced by querying a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(mouseArea->functionMap.mouseDown
       && pointInRect(x, y, &mouseArea->rect))
    {
      mouseArea->functionMap.mouseDown(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
}


void guihckContextMouseUp(guihckContext* ctx, float x, float y, int button)
{
  /* Should be replaced by querying a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(mouseArea->functionMap.mouseUp
       && pointInRect(x, y, &mouseArea->rect))
    {
      mouseArea->functionMap.mouseUp(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
}


void guihckContextMouseMove(guihckContext* ctx, float sx, float sy, float dx, float dy)
{
  /* Should be replaced by querying the segment from a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(!mouseArea->functionMap.mouseMove && !mouseArea->functionMap.mouseEnter && !mouseArea->functionMap.mouseExit)
      continue;

    bool s = pointInRect(sx, sy, &mouseArea->rect);
    bool d = pointInRect(dx, dy, &mouseArea->rect);

    if(s && d)
    {
      if(mouseArea->functionMap.mouseMove)
        mouseArea->functionMap.mouseMove(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
    else if(s)
    {
      if(mouseArea->functionMap.mouseExit)
        mouseArea->functionMap.mouseExit(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
    else if(d)
    {
      if(mouseArea->functionMap.mouseEnter)
        mouseArea->functionMap.mouseEnter(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
  }
}


guihckMouseAreaId guihckMouseAreaNew(guihckContext* ctx, guihckElementId elementId, guihckMouseAreaFunctionMap functionMap)
{
  _guihckMouseArea mouseArea;
  mouseArea.elementId = elementId;
  mouseArea.rect.x = 0;
  mouseArea.rect.y = 0;
  mouseArea.rect.w = 0;
  mouseArea.rect.h = 0;
  mouseArea.functionMap = functionMap;
  guihckMouseAreaId id;
  chckPoolAdd(ctx->mouseAreas, &mouseArea, &id);
  return id;
}


void guihckMouseAreaRemove(guihckContext* ctx, guihckMouseAreaId mouseAreaId)
{
  chckPoolRemove(ctx->mouseAreas, mouseAreaId);
}


void guihckMouseAreaRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float x, float y, float width, float height)
{
  _guihckMouseArea* mouseArea = (_guihckMouseArea*) chckPoolGet(ctx->mouseAreas, mouseAreaId);
  if(mouseArea)
  {
    mouseArea->rect.x = x;
    mouseArea->rect.y = y;
    mouseArea->rect.w = width;
    mouseArea->rect.h = height;
  }
}


void guihckMouseAreaGetRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float* x, float* y, float* width, float* height)
{
  _guihckMouseArea* mouseArea = (_guihckMouseArea*) chckPoolGet(ctx->mouseAreas, mouseAreaId);
  if(mouseArea)
  {
    if(x) *x = mouseArea->rect.x;
    if(y) *y = mouseArea->rect.y;
    if(width) *width = mouseArea->rect.w;
    if(height) *height = mouseArea->rect.h;
  }
}

char pointInRect(float x, float y, const _guihckRect* r)
{
  return x >= r->x && x <= r->x + r->w && y >= r->y && y <= r->y + r->h;
}

