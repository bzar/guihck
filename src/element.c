#include "internal.h"

#include <assert.h>

static void _guihckElementUpdateChildrenProperty(guihckContext* ctx, guihckElementId elementId);
static void _guihckRemoveListeners(guihckContext* ctx, chckIterPool* pool);
static bool _guihckPropertyIsAnAlias(SCM value);
static bool _guihckPropertyIsBound(SCM value);
static void _guihckElementPropertyNotifyListeners(guihckContext* ctx, _guihckProperty* property);
static void _guihckPropertyAliasListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);
static void _guihckPropertyCreateAlias(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property);
static void _guihckPropertyCreateBind(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property);
static void _guihckPropertyCreate(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property);
static void _guihckPropertyListenerFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);
static void _guihckPropertyAliasFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);
static void _guihckPropertyBindListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);
static void _guihckOrderListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);

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
  }

  if(type->functionMap.init)
    type->functionMap.init(ctx, id, element.data);

  _guihckElementUpdateChildrenProperty(ctx, id);
  if(parent)
  {
    _guihckElementUpdateChildrenProperty(ctx, parentId);
    guihckElementAddListener(ctx, parentId, id, "order", _guihckOrderListenerCallback, NULL, NULL);
  }

  guihckElementProperty(ctx, id, "focus", SCM_BOOL_F);
  return id;
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
    _guihckRemoveListeners(ctx, element->listened);
    chckIterPoolFree(element->listened);
  }

  _guihckProperty* property;
  chckHashTableIterator pIter = {NULL, 0};
  while((property = chckHashTableIter(element->properties, &pIter)))
  {
    /* Remove property listeners for listeners */
    if(property->listeners)
    {
      _guihckRemoveListeners(ctx, property->listeners);
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
      while((bound = chckIterPoolIter(property->bind.bound, &bIter)))
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

  /* If was focused, focus to root */
  if(ctx->focused == elementId)
    ctx->focused = ctx->rootElementId;
}


SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char* key)
{
#if 0
  printf("guihckElementGetProperty %d %s\n", (int) elementId, key);
#endif
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckProperty* prop = chckHashTableStrGet(element->properties, key);

  if(!prop)
  {
    return SCM_UNDEFINED;
  }

#if 0
  if(scm_is_eq(prop->value, SCM_UNDEFINED)) printf(" --> UNDEFINED\n");
  else printf(" --> %s\n", scm_to_utf8_string(scm_object_to_string(prop->value, SCM_UNDEFINED)));
#endif

  return prop->value;
}

void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
#if 0
  char* valueStr = scm_to_utf8_string(scm_object_to_string(value, SCM_UNDEFINED));
  printf("guihckElementProperty %s %d %s %s\n", ((_guihckElementType*)chckPoolGet(ctx->elementTypes, element->type))->name, (int) elementId, key, valueStr);
  free(valueStr);
#endif
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
    while((bound = chckIterPoolIter(existing->bind.bound, &iter)))
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
  assert(childId && "Element does not have requested child");
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

bool guihckElementGetVisible(guihckContext* ctx, guihckElementId elementId)
{
  SCM visible = guihckElementGetProperty(ctx, elementId, "visible");
  if(scm_is_eq(visible, SCM_UNDEFINED))
  {
    guihckElementVisible(ctx, elementId, true);
    return true;
  }
  else
  {
    return scm_to_bool(visible);
  }
}

void guihckElementVisible(guihckContext* ctx, guihckElementId elementId, bool value)
{
  guihckElementProperty(ctx, elementId, "visible", scm_from_bool(value));
}

/*
 * Private
 */


void _guihckElementUpdateChildrenProperty(guihckContext* ctx, guihckElementId elementId)
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

void _guihckRemoveListeners(guihckContext* ctx, chckIterPool* pool)
{
  if(chckIterPoolCount(pool) > 0)
  {
    /* Copy listener ids before removing as modifying the pool may break iterators */
    size_t listenedCount;
    guihckPropertyListenerId* listenerIdsOrig = chckIterPoolToCArray(pool, &listenedCount);
    guihckPropertyListenerId* listenerIds = calloc(listenedCount, sizeof(guihckPropertyListenerId));
    memcpy(listenerIds, listenerIdsOrig, listenedCount * sizeof(guihckPropertyListenerId));

    unsigned int i;
    for(i = 0; i < listenedCount; ++i)
    {
      guihckElementRemoveListener(ctx, listenerIds[i]);
    }
    free(listenerIds);
  }
}

bool _guihckPropertyIsAnAlias(SCM value)
{
  return scm_is_pair(value) && scm_is_symbol(SCM_CAR(value))
      && scm_is_eq(SCM_CAR(value), scm_string_to_symbol(scm_from_utf8_string("alias")));
}

bool _guihckPropertyIsBound(SCM value)
{
  return scm_is_pair(value) && scm_is_symbol(SCM_CAR(value))
      && scm_is_eq(SCM_CAR(value), scm_string_to_symbol(scm_from_utf8_string("bind")));
}

void _guihckElementPropertyNotifyListeners(guihckContext* ctx, _guihckProperty* property)
{
  if(property->listeners)
  {
    chckPoolIndex iter = 0;
    guihckPropertyListenerId* listenerId;
    while((listenerId = chckIterPoolIter(property->listeners, &iter)))
    {
      _guihckPropertyListener* listener = chckPoolGet(ctx->propertyListeners, *listenerId);
      listener->callback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, property->value, listener->data);
    }
  }
}

void _guihckPropertyAliasListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) listenedId;
  (void) property;

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

void _guihckPropertyCreateAlias(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
{
  SCM elementExpression = SCM_CADR(value);
  SCM propertyNameExpression = SCM_CADDR(value);
  guihckStackPushElement(ctx, elementId);
  SCM elementValue = elementExpression;
  SCM propertyNameValue = propertyNameExpression;
  guihckStackPopElement(ctx);

  guihckElementId targetId = scm_to_uint64(elementValue);
  char* targetPropertyName = scm_to_utf8_string(scm_symbol_to_string(propertyNameValue));
  property->alias.listenerId = guihckElementAddListener(ctx, elementId, targetId, targetPropertyName,
                                                        _guihckPropertyAliasListenerCallback, strdup(propertyName),
                                                        _guihckPropertyAliasFreeCallback);
  property->value = guihckElementGetProperty(ctx, targetId, targetPropertyName);
  if(!scm_is_eq(property->value, SCM_UNDEFINED))
  {
    scm_gc_protect_object(property->value);
  }
  free(targetPropertyName);
}

void _guihckPropertyBindListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) listenedId;
  (void) property;

  _guihckBoundPropertyRef* ref = data;
  guihckElement* listener = chckPoolGet(ctx->elements, listenerId);
#if 0
  char* valueStr = scm_to_utf8_string(scm_object_to_string(value, SCM_UNDEFINED));
  const char* listenerType = ((_guihckElementType*) chckPoolGet(ctx->elementTypes, listener->type))->name;
  printf("_guihckPropertyBindListenerCallback %d %s %d %s %s\n", (int) listenerId, listenerType, (int) listenedId, property, valueStr);
  free(valueStr);
#endif

  _guihckProperty* listenerProperty = chckHashTableStrGet(listener->properties, ref->propertyName);

  guihckStackPushElement(ctx, listenerId);
  SCM paramsVector = scm_c_make_vector(chckIterPoolCount(listenerProperty->bind.bound), SCM_UNDEFINED);
  bool hasUndefined = false;

  chckPoolIndex iter = 0;
  _guihckBoundProperty* bound;
  while((bound = chckIterPoolIter(listenerProperty->bind.bound, &iter)))
  {
    if(bound->index == ref->index)
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
    }

    scm_c_vector_set_x(paramsVector, bound->index, bound->value);
  }

  SCM newValue = SCM_UNDEFINED;
  if(!hasUndefined)
  {
    SCM expression = scm_cons(listenerProperty->bind.function, scm_vector_to_list(paramsVector));
#if 0
  char* paramsStr = scm_to_utf8_string(scm_object_to_string(paramsVector, SCM_UNDEFINED));
  printf("PARAMS VECTOR: %s\n", paramsStr);
  free(paramsStr);
  char* expressionStr = scm_to_utf8_string(scm_object_to_string(expression, SCM_UNDEFINED));
  printf("LISTENER EXPR: %s\n", expressionStr);
  free(expressionStr);
#endif
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

void _guihckPropertyCreateBind(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
{
  SCM boundList = SCM_CADR(value);
  SCM function = SCM_CADDR(value);

  guihckStackPushElement(ctx, elementId);

  SCM boundVector = scm_vector(boundList);
  char* paramsStr = scm_to_utf8_string(scm_object_to_string(boundVector, SCM_UNDEFINED));
  free(paramsStr);

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
    _guihckBoundPropertyRef* ref = calloc(1, sizeof(_guihckBoundPropertyRef));
    ref->propertyName = strdup(propertyName);
    ref->index = i;


    b.listenerId = guihckElementAddListener(ctx, elementId, boundElementId, boundPropertyName, _guihckPropertyBindListenerCallback, ref,
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
    char* expressionStr = scm_to_utf8_string(scm_object_to_string(expression, SCM_UNDEFINED));
    free(expressionStr);

    property->value = guihckContextExecuteExpression(ctx, expression);
    scm_gc_protect_object(property->value);
  }
  guihckStackPopElement(ctx);
}

void _guihckPropertyCreate(guihckContext* ctx, guihckElementId elementId, const char* propertyName, SCM value, _guihckProperty* property)
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


void _guihckPropertyListenerFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) ctx;
  (void) listenerId;
  (void) listenedId;
  (void) property;
  (void) value;

  if(data)
  {
    _guihckBoundPropertyRef* ref = data;
    free(ref->propertyName);
    free(ref);
  }
}

void _guihckPropertyAliasFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) ctx;
  (void) listenerId;
  (void) listenedId;
  (void) property;
  (void) value;

  if(data)
  {
    free(data);
  }
}

void _guihckOrderListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) property;
  (void) data;

  int myOrder = scm_is_integer(value) ? scm_to_int32(value) : 0;

  guihckElement* parent = chckPoolGet(ctx->elements, listenerId);

  size_t nn;
  guihckElementId* children = chckIterPoolToCArray(parent->children, &nn);

  int i;
  int n = nn;

  /* Find this child */
  for(i = 0; i < n && children[i] != listenedId; ++i);

  /* Move forward until order is same or less */
  for(i = i + 1; i < n; ++i)
  {
    SCM orderScm = guihckElementGetProperty(ctx, children[i], "order");
    int order = scm_is_integer(orderScm) ? scm_to_int32(orderScm) : 0;
    if(order > myOrder)
    {
      children[i - 1] = children[i];
      children[i] = listenedId;
    }
    else
    {
      break;
    }
  }

  /* Move backward until order is same or greater */
  for(i = i - 2; i >= 0; --i)
  {
    SCM orderScm = guihckElementGetProperty(ctx, children[i], "order");
    int order = scm_is_integer(orderScm) ? scm_to_int32(orderScm) : 0;
    if(order <= myOrder)
    {
      children[i + 1] = children[i];
      children[i] = listenedId;
    }
    else
    {
      break;
    }
  }

  _guihckElementUpdateChildrenProperty(ctx, listenerId);
}
