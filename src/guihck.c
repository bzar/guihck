#include "guihck.h"
#include "guihckGuile.h"
#include "pool.h"
#include "lut.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define GUIHCK_NO_PARENT -1

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
  guihckPropertyListenerFreeCallback freeCallback;
} _guihckPropertyListener;

typedef enum _guihckPropertyType { GUIHCK_PROPERTY_VALUE, GUIHCK_PROPERTY_ALIAS, GUIHCK_PROPERTY_BIND } _guihckPropertyType;
typedef struct _guihckBoundProperty
{
  size_t index;
  guihckPropertyListenerId listenerId;
  SCM value;
} _guihckBoundProperty;

typedef struct _guihckProperty
{
  _guihckPropertyType type;
  SCM value;
  chckIterPool* listeners;
  union
  {
    struct
    {
      guihckPropertyListenerId listenerId;
    } alias;
    struct
    {
      SCM function;
      chckIterPool* bound; /* _guihckBoundProperty for each bound property */
    } bind;
  };

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
static void _guihckPropertyListenerFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);

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
  _guihckPropertyListener* listener;
  while(listener = chckPoolIter(ctx->propertyListeners, &iter))
  {
    if(listener->freeCallback)
      listener->freeCallback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, SCM_UNDEFINED, listener->data);
    free(listener->propertyName);
  }
  chckPoolFree(ctx->propertyListeners);

  iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
    if(type->functionMap.destroy)
      type->functionMap.destroy(ctx, iter - 1, current->data); /* id = iter - 1 */

    if(current->listened)
      chckIterPoolFree(current->listened);

    _guihckProperty* property;
    chckHashTableIterator pIter = {NULL, 0};
    while(property = chckHashTableIter(current->properties, &pIter))
    {
      if(property->listeners)
        chckIterPoolFree(property->listeners);

      if(property->type == GUIHCK_PROPERTY_BIND)
      {
        chckIterPoolFree(property->bind.bound);
      }
    }

    if(current->properties)
      chckHashTableFree(current->properties);
    if(current->children)
      chckIterPoolFree(current->children);
    if(current->data)
      free(current->data);
  }

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

static void _guihckElementUpdateChildrenProperty(guihckContext* ctx, guihckElementId elementId)
{
  size_t numChildren = guihckElementGetChildCount(ctx, elementId);
  int i;
  SCM children = SCM_EOL;
  for(i = numChildren - 1; i >= 0; --i)
  {
    guihckElementId childId = guihckElementGetChild(ctx, elementId, i);
    children = scm_cons(scm_from_uint64(childId), children);
  }

  guihckElementProperty(ctx, elementId, "children", children);
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

  _guihckElementUpdateChildrenProperty(ctx, id);
  if(parent)
  {
    _guihckElementUpdateChildrenProperty(ctx, parentId);
  }

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
  guihckElementId parentId = guihckElementGetParent(ctx, elementId);
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

  _guihckProperty* property;
  chckHashTableIterator pIter = {NULL, 0};
  while(property = chckHashTableIter(element->properties, &pIter))
  {
    /* Remove property listeners for listeners */
    if(property->listeners)
    {
      removeListeners(ctx, property->listeners);
      chckIterPoolFree(property->listeners);
    }

    if(property->type == GUIHCK_PROPERTY_ALIAS)
    {
      guihckElementRemoveListener(ctx, property->alias.listenerId);
    }
    else if(property->type == GUIHCK_PROPERTY_BIND)
    {
      _guihckBoundProperty* bound;
      chckPoolIndex bIter = 0;
      while(bound = chckIterPoolIter(property->bind.bound, &bIter))
      {
        guihckElementRemoveListener(ctx, bound->listenerId);
        bound->value = SCM_UNDEFINED;
      }
      chckIterPoolFree(property->bind.bound);
      scm_gc_unprotect_object(property->bind.function);
      property->bind.function = SCM_UNDEFINED;
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

  guihckElement* parent = chckPoolGet(ctx->elements, parentId);
  if(parent)
  {
    iter = 0;
    while((current = chckIterPoolIter(parent->children, &iter)) && *current != elementId);
    if(current)
      chckIterPoolRemove(parent->children, iter - 1);

    _guihckElementUpdateChildrenProperty(ctx, parentId);
  }
}


SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char* key)
{
#if 0
  printf("guihckElementGetProperty %d %s\n", elementId, key);
#endif
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckProperty* prop = chckHashTableStrGet(element->properties, key);

  if(!prop)
  {
    return SCM_UNDEFINED;
  }

#if 0
  printf(" --> %s\n", scm_to_utf8_string(scm_object_to_string(prop->value, SCM_UNDEFINED)));
#endif

  return prop->value;
}

static bool _guihckPropertyIsAnAlias(SCM value)
{
  return scm_is_pair(value) && scm_is_symbol(SCM_CAR(value))
      && scm_is_eq(SCM_CAR(value), scm_string_to_symbol(scm_from_utf8_string("alias")));
}

static bool _guihckPropertyIsBound(SCM value)
{
  return scm_is_pair(value) && scm_is_symbol(SCM_CAR(value))
      && scm_is_eq(SCM_CAR(value), scm_string_to_symbol(scm_from_utf8_string("bind")));
}

static void _guihckElementPropertyNotifyListeners(guihckContext* ctx, _guihckProperty* property)
{
  if(property->listeners)
  {
    chckPoolIndex iter = 0;
    guihckPropertyListenerId* listenerId;
    while(listenerId = chckIterPoolIter(property->listeners, &iter))
    {
      _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, *listenerId);
      listener->callback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, property->value, listener->data);
    }
  }
}

static void _guihckPropertyAliasListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  char* key = data;
  guihckElement* listener = chckPoolGet(ctx->elements, listenerId);
  _guihckProperty* listenerProperty = chckHashTableStrGet(listener->properties, key);

  if(!scm_is_eq(listenerProperty->value, SCM_UNDEFINED))
  {
    scm_gc_unprotect_object(listenerProperty->value);
  }

  listenerProperty->value = value;

  if(!scm_is_eq(listenerProperty->value, SCM_UNDEFINED))
  {
    scm_gc_protect_object(listenerProperty->value);
  }

  _guihckElementPropertyNotifyListeners(ctx, listenerProperty);
}

static void _guihckPropertyCreateAlias(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
{
  SCM elementExpression = SCM_CADR(value);
  SCM propertyNameExpression = SCM_CADDR(value);
  guihckStackPushElement(ctx, elementId);
  SCM elementValue = guihckContextExecuteExpression(ctx, elementExpression);
  SCM propertyNameValue = guihckContextExecuteExpression(ctx, propertyNameExpression);
  guihckStackPopElement(ctx);

  guihckElementId targetId = scm_to_uint64(elementValue);
  char* targetPropertyName = scm_to_utf8_string(scm_symbol_to_string(propertyNameValue));

  property->alias.listenerId = guihckElementAddListener(ctx, elementId, targetId, targetPropertyName,
                                                        _guihckPropertyAliasListenerCallback, strdup(propertyName),
                                                        _guihckPropertyListenerFreeCallback);
  property->value = guihckElementGetProperty(ctx, targetId, targetPropertyName);
  if(!scm_is_eq(property->value, SCM_UNDEFINED))
  {
    scm_gc_protect_object(property->value);
  }
  free(targetPropertyName);
}

static void _guihckPropertyBindListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  char* key = data;
  guihckElement* listener = chckPoolGet(ctx->elements, listenerId);
  _guihckProperty* listenerProperty = chckHashTableStrGet(listener->properties, key);

  guihckStackPushElement(ctx, listenerId);
  SCM paramsVector = scm_c_make_vector(chckIterPoolCount(listenerProperty->bind.bound), SCM_UNDEFINED);
  bool hasUndefined = false;

  chckPoolIndex iter = 0;
  _guihckBoundProperty* bound;
  size_t i;
  for(i = 0; bound = chckIterPoolIter(listenerProperty->bind.bound, &iter); ++i)
  {
    if(i == bound->index)
    {
      if(!scm_is_eq(bound->value, SCM_UNDEFINED))
      {
        scm_gc_unprotect_object(bound->value);
      }

      bound->value = value;

      if(!scm_is_eq(bound->value, SCM_UNDEFINED))
      {
        scm_gc_protect_object(bound->value);
      }
    }

    if(scm_is_eq(bound->value, SCM_UNDEFINED))
    {
      hasUndefined = true;
      break;
    }

    scm_c_vector_set_x(paramsVector, i, bound->value);
  }

  SCM newValue = SCM_UNDEFINED;
  if(!hasUndefined)
  {
    SCM expression = scm_cons(listenerProperty->bind.function, scm_vector_to_list(paramsVector));
    newValue = guihckContextExecuteExpression(ctx, expression);
    if(!scm_is_eq(newValue, SCM_UNDEFINED))
    {
      scm_gc_protect_object(newValue);
    }
  }

  if(scm_is_false(scm_equal_p(listenerProperty->value, newValue)))
  {
    listenerProperty->value = newValue;
    _guihckElementPropertyNotifyListeners(ctx, listenerProperty);
  }

  guihckStackPopElement(ctx);
}

static void _guihckPropertyCreateBind(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
{
  SCM boundListExpression = SCM_CADR(value);
  SCM functionExpression = SCM_CADDR(value);

  guihckStackPushElement(ctx, elementId);
  SCM boundVector = scm_vector(guihckContextExecuteExpression(ctx, boundListExpression));
  SCM function = guihckContextExecuteExpression(ctx, functionExpression);

  property->bind.function = function;
  scm_gc_protect_object(property->bind.function);

  size_t numBound = scm_c_vector_length(boundVector);
  property->bind.bound = chckIterPoolNew(8, numBound, sizeof(_guihckBoundProperty));
  SCM paramsVector = scm_c_make_vector(numBound, SCM_UNDEFINED);
  bool hasUndefined = false;
  size_t i;
  for(i = 0; i < numBound; ++i)
  {
    SCM bound = scm_c_vector_ref(boundVector, i);
    SCM boundElement = SCM_CAR(bound);
    SCM boundProperty = SCM_CDR(bound);

    guihckElementId boundElementId = scm_to_uint64(boundElement);
    char* boundPropertyName = scm_to_utf8_string(scm_symbol_to_string(boundProperty));

    _guihckBoundProperty b;
    b.index = i;
    b.listenerId = guihckElementAddListener(ctx, elementId, boundElementId, boundPropertyName, _guihckPropertyBindListenerCallback, strdup(propertyName),
                                            _guihckPropertyListenerFreeCallback);

    b.value = guihckElementGetProperty(ctx, boundElementId, boundPropertyName);

    if(!scm_is_eq(b.value, SCM_UNDEFINED))
    {
      scm_gc_protect_object(b.value);
    }
    else
    {
      hasUndefined = true;
    }
    SCM param = scm_is_pair(b.value) ? scm_list_2(scm_sym_quote, b.value) : b.value;
    scm_c_vector_set_x(paramsVector, i,  param);
    chckIterPoolAdd(property->bind.bound, &b, NULL);
  }

  if(hasUndefined)
  {
    property->value = SCM_UNDEFINED;
  }
  else
  {
    SCM expression = scm_cons(property->bind.function, scm_vector_to_list(paramsVector));
    property->value = guihckContextExecuteExpression(ctx, expression);
    scm_gc_protect_object(property->value);
  }
  guihckStackPopElement(ctx);
}

static void _guihckPropertyCreate(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
{
  property->type =
      _guihckPropertyIsAnAlias(value) ? GUIHCK_PROPERTY_ALIAS :
      _guihckPropertyIsBound(value) ? GUIHCK_PROPERTY_BIND :
      GUIHCK_PROPERTY_VALUE;
  property->listeners = NULL;
  /* Set value contents based on type */
  switch(property->type)
  {
    case GUIHCK_PROPERTY_ALIAS:
    {
      _guihckPropertyCreateAlias(ctx, elementId, propertyName, value, property);
      break;
    }
    case GUIHCK_PROPERTY_BIND:
    {
      _guihckPropertyCreateBind(ctx, elementId, propertyName, value, property);
      break;
    }
    case GUIHCK_PROPERTY_VALUE:
    {
      if(!scm_is_eq(value, SCM_UNDEFINED))
        scm_gc_protect_object(value);
      property->value = value;
      break;
    }
  }
}

void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value)
{
#if 0
  char* valueStr = scm_to_utf8_string(scm_object_to_string(value, SCM_UNDEFINED));
  printf("guihckElementProperty %d %s %s\n", elementId, key, valueStr);
  free(valueStr);
#endif
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckProperty* existing = chckHashTableStrGet(element->properties, key);

  /* Property is an alias, delegate and return */
  if(existing && existing->type == GUIHCK_PROPERTY_ALIAS)
  {
    guihckPropertyListenerId plid = existing->alias.listenerId;
    _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, plid);
    guihckElementProperty(ctx, listener->listenedId, listener->propertyName, value);
    return;
  }

  bool isNewValue = existing && (existing->type == GUIHCK_PROPERTY_BIND || scm_is_false(scm_equal_p(existing->value, value)));

  /* No action if existing value is equal */
  if(existing && !isNewValue)
    return;

  /* Previous value is a bind, remove it before setting new value */
  if(existing && existing->type == GUIHCK_PROPERTY_BIND)
  {
    chckPoolIndex iter = 0;
    _guihckBoundProperty* bound;
    while(bound = chckIterPoolIter(existing->bind.bound, &iter))
    {
      if(!scm_is_eq(bound->value, SCM_UNDEFINED))
      {
        scm_gc_unprotect_object(bound->value);
      }
      guihckElementRemoveListener(ctx, bound->listenerId);
    }
    chckIterPoolFree(existing->bind.bound);
    scm_gc_unprotect_object(existing->bind.function);
    existing->bind.function = SCM_UNDEFINED;
  }


  /* Create new value for the property */
  _guihckProperty property;
  _guihckPropertyCreate(ctx, elementId, key, value, &property);

  if(!existing)
  {
    /* Create new property */
    chckHashTableStrSet(element->properties, key, &property, sizeof(_guihckProperty));
    guihckElementDirty(ctx, elementId);
  }
  else if(isNewValue)
  {
    /* Update existing property */
    if(!scm_is_eq(existing->value, SCM_UNDEFINED))
    {
      scm_gc_unprotect_object(existing->value);
    }

    switch(property.type)
    {
      case GUIHCK_PROPERTY_ALIAS:
        existing->type = property.type;
        existing->value = property.value;
        existing->alias = property.alias;
        break;
      case GUIHCK_PROPERTY_BIND:
        existing->type = property.type;
        existing->value = property.value;
        existing->bind = property.bind;
        break;
      case GUIHCK_PROPERTY_VALUE:
        existing->type = property.type;
        existing->value = property.value;
        break;
      default:
        assert(false && "Unknown property type");
    }

    _guihckElementPropertyNotifyListeners(ctx, existing);
    guihckElementDirty(ctx, elementId);
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
                                                  const char* propertyName, guihckPropertyListenerCallback callback, void* data,
                                                  guihckPropertyListenerFreeCallback freeCallback)
{
  _guihckPropertyListener propertyListener;
  propertyListener.listenerId = listenerId;
  propertyListener.listenedId = listenedId;
  propertyListener.propertyName = strdup(propertyName);
  propertyListener.callback = callback;
  propertyListener.data = data;
  propertyListener.freeCallback = freeCallback;
  guihckPropertyListenerId id;
  chckPoolAdd(ctx->propertyListeners, &propertyListener, &id);

  guihckElement* listenedElement = chckPoolGet(ctx->elements, listenedId);
  _guihckProperty* property = chckHashTableStrGet(listenedElement->properties, propertyName);
  if(!property)
  {
    guihckElementProperty(ctx, listenedId, propertyName, SCM_UNDEFINED);
    property = chckHashTableStrGet(listenedElement->properties, propertyName);
  }
  if(!property->listeners)
    property->listeners = chckIterPoolNew(4, 4, sizeof(guihckPropertyListenerId));
  chckIterPoolAdd(property->listeners, &id, NULL);

  guihckElement* listenerElement = chckPoolGet(ctx->elements, listenerId);
  if(!listenerElement->listened)
    listenerElement->listened = chckIterPoolNew(4, 4, sizeof(guihckPropertyListenerId));

  chckIterPoolAdd(listenerElement->listened, &id, NULL);

  return id;
}

void guihckElementRemoveListener(guihckContext* ctx, guihckPropertyListenerId propertyListenerId)
{
  _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, propertyListenerId);
  if(!listener)
    return;

  guihckElement* listenedElement = chckPoolGet(ctx->elements, listener->listenedId);
  _guihckProperty* property = chckHashTableStrGet(listenedElement->properties, listener->propertyName);

  if(listener->freeCallback)
    listener->freeCallback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, property->value, listener->data);

  chckPoolIndex iter = 0;
  guihckPropertyListenerId* id;
  while((id = chckIterPoolIter(property->listeners, &iter)) && *id != propertyListenerId);

  if(id)
    chckIterPoolRemove(property->listeners, iter - 1);

  guihckElement* listenerElement = chckPoolGet(ctx->elements, listener->listenerId);

  if(listenerElement)
  {
    iter = 0;
    id = NULL;
    while((id = chckIterPoolIter(listenerElement->listened, &iter)) && *id != propertyListenerId);

    if(id)
      chckIterPoolRemove(listenerElement->listened, iter - 1);

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
  if(parentId != GUIHCK_NO_PARENT);
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

static void _guihckPropertyListenerFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  if(data)
    free(data);
}

