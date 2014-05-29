#include "guihck.h"
#include "guihckGuile.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

static void _guihckContextAddDefaultKeybindings(guihckContext* ctx);

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

  guihckElementTypeFunctionMap rootElementFunctionMap = { NULL, NULL, NULL, NULL, NULL, NULL };
  guihckElementTypeId rootTypeId = guihckElementTypeAdd(ctx, "root", rootElementFunctionMap, 0);
  ctx->rootElementId = guihckElementNew(ctx, rootTypeId, GUIHCK_NO_PARENT);
  guihckElementProperty(ctx, ctx->rootElementId, "id", scm_from_utf8_string("root"));
  guihckStackPushElement(ctx, ctx->rootElementId);
  ctx->focused = ctx->rootElementId;

  ctx->keyCodesByName = chckHashTableNew(32);
  ctx->keyNamesByCode = chckHashTableNew(32);

  _guihckContextAddDefaultKeybindings(ctx);

  ctx->time = 0;

  return ctx;
}


void guihckContextFree(guihckContext* ctx)
{
  {
    chckPoolIndex iter = 0;
    _guihckPropertyListener* listener;
    while((listener = chckPoolIter(ctx->propertyListeners, &iter)))
    {
      if(listener->freeCallback)
        listener->freeCallback(ctx, listener->listenerId, listener->listenedId, listener->propertyName, SCM_UNDEFINED, listener->data);
      free(listener->propertyName);
    }
    chckPoolFree(ctx->propertyListeners);
  }

  {
    chckPoolIndex iter = 0;
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
      while((property = chckHashTableIter(current->properties, &pIter)))
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
  }

  chckPoolFree(ctx->mouseAreas);
  chckPoolFree(ctx->elements);
  chckHashTableFree(ctx->elementTypesByName);
  chckPoolFree(ctx->elementTypes);

  {
    char** keyName;
    chckHashTableIterator knIter = {NULL, 0};
    while((keyName = chckHashTableIter(ctx->keyNamesByCode, &knIter)))
    {
      free(*keyName);
    }
  }

  chckHashTableFree(ctx->keyNamesByCode);
  chckHashTableFree(ctx->keyCodesByName);

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
void guihckContextKeyboardFocus(guihckContext* ctx, guihckElementId elementId)
{
  guihckElementProperty(ctx, ctx->focused, "focus", SCM_BOOL_F);
  ctx->focused = elementId;
  guihckElementProperty(ctx, elementId, "focus", SCM_BOOL_T);
}

guihckElementId guihckContextGetKeyboardFocus(guihckContext* ctx)
{
  return ctx->focused;
}

void guihckContextAddKeyBinding(guihckContext* ctx, guihckKey keyCode, const char* keyName)
{
  if(chckHashTableGet(ctx->keyNamesByCode, keyCode) == NULL)
  {
    char* keyNameCopy = strdup(keyName);
    chckHashTableSet(ctx->keyNamesByCode, keyCode, &keyNameCopy, sizeof(char*));
  }

  if(chckHashTableStrGet(ctx->keyCodesByName, keyName) == NULL)
  {
    chckHashTableStrSet(ctx->keyCodesByName, keyName, &keyCode, sizeof(guihckKey));
  }
}

const char* guihckContextGetKeyName(guihckContext* ctx, guihckKey keyCode)
{
  const char** result = chckHashTableGet(ctx->keyNamesByCode, keyCode);
  return result ? *result : NULL;
}

guihckKey guihckContextGetKeyCode(guihckContext* ctx, const char* keyName)
{
  guihckKey* result = chckHashTableStrGet(ctx->keyCodesByName, keyName);
  return result ? *result : GUIHCK_KEY_UNKNOWN;
}

void guihckContextKeyboardKey(guihckContext* ctx, guihckKey key, int scancode, guihckKeyAction action, guihckKeyMods mods)
{
  guihckElementId id = ctx->focused;

  SCM keyScm = scm_from_int32(key);
  SCM scancodeScm = scm_from_int32(scancode);
  SCM actionScm =
      action == GUIHCK_KEY_PRESS ? scm_from_utf8_symbol("press") :
      action == GUIHCK_KEY_RELEASE ? scm_from_utf8_symbol("release") :
      action == GUIHCK_KEY_PRESS ? scm_from_utf8_symbol("repeat") :
      scm_from_utf8_symbol("unknown");
  actionScm = scm_list_2(scm_sym_quote, actionScm);

  SCM modsScm = SCM_EOL;
  if(mods & GUIHCK_MOD_SHIFT)
    modsScm = scm_cons(scm_from_utf8_symbol("shift"), modsScm);
  if(mods & GUIHCK_MOD_ALT)
    modsScm = scm_cons(scm_from_utf8_symbol("alt"), modsScm);
  if(mods & GUIHCK_MOD_CONTROL)
    modsScm = scm_cons(scm_from_utf8_symbol("control"), modsScm);
  if(mods & GUIHCK_MOD_SUPER)
    modsScm = scm_cons(scm_from_utf8_symbol("super"), modsScm);
  modsScm = scm_list_2(scm_sym_quote, modsScm);

  /* Move up the element tree until someone handles the event */
  bool handled = false;
  while(!handled && id != GUIHCK_NO_PARENT)
  {
    /* First attempt to call native handler */
    guihckElement* element = chckPoolGet(ctx->elements, id);
    _guihckElementType* elementType = chckPoolGet(ctx->elementTypes, element->type);
    if(elementType->functionMap.keyChar)
    {
      handled = elementType->functionMap.keyEvent(ctx, id, key, scancode, action, mods, element->data);
    }

    /* Second try to call a script handler */
    if(!handled)
    {
      SCM handler = guihckElementGetProperty(ctx, id, "on-key");
      if(scm_is_true(scm_procedure_p(handler)))
      {
        SCM expression = scm_list_5(handler, keyScm, scancodeScm, actionScm, modsScm);
        guihckStackPushElement(ctx, id);
        SCM result = guihckContextExecuteExpression(ctx, expression);
        guihckStackPopElement(ctx);
        handled = scm_is_eq(result, SCM_BOOL_T);
      }
    }

    id = guihckElementGetParent(ctx, id);
  }

}

void guihckContextKeyboardChar(guihckContext* ctx, unsigned int codepoint)
{
  guihckElementId id = ctx->focused;
  SCM codepointChar = scm_integer_to_char(scm_from_uint32(codepoint));

  /* Move up the element tree until someone handles the event */
  bool handled = false;
  while(!handled && id != GUIHCK_NO_PARENT)
  {
    /* First attempt to call native handler */
    guihckElement* element = chckPoolGet(ctx->elements, id);
    _guihckElementType* elementType = chckPoolGet(ctx->elementTypes, element->type);
    if(elementType->functionMap.keyChar)
    {
      handled = elementType->functionMap.keyChar(ctx, id, codepoint, element->data);
    }

    /* Second try to call a script handler */
    if(!handled)
    {
      SCM handler = guihckElementGetProperty(ctx, id, "on-char");
      if(scm_is_true(scm_procedure_p(handler)))
      {
        SCM expression = scm_list_2(handler, codepointChar);
        guihckStackPushElement(ctx, id);
        SCM result = guihckContextExecuteExpression(ctx, expression);
        guihckStackPopElement(ctx);

        handled = scm_is_eq(result, SCM_BOOL_T);
      }
    }

    id = guihckElementGetParent(ctx, id);
  }
}

void guihckContextTime(guihckContext* ctx, double time)
{
  assert(time >= ctx->time && "Backwards time travel not allowed");
  ctx->time = time;
}

double guihckContextGetTime(guihckContext* ctx)
{
  return ctx->time;
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

  SCM result = SCM_UNDEFINED;

  if((file = fopen(path, "rb")))
  {
    /* Read file into memory */
    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* contents = calloc(pos, 1);
    fread(contents, pos, 1, file);
    fclose(file);

    result = guihckContextExecuteScript(ctx, contents);
    free(contents);
  }

  return result;
}

void _guihckContextAddDefaultKeybindings(guihckContext* ctx)
{
  size_t n;
  const guihckDefaultKeyBinding* bindings = guihckGetDefaultKeyBindings(&n);

  unsigned int i;
  for(i = 0; i < n; ++i)
  {
    const guihckDefaultKeyBinding* b = &(bindings[i]);
    guihckContextAddKeyBinding(ctx, b->code, b->name);
  }
}
