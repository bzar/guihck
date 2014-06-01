#include "guihckGuile.h"
#include "internal.h"
#include "guihckGuileDefaultScm.h"
#include <assert.h>

typedef struct _guihckGuileContext
{
  guihckContext* ctx;
  int ctxRefs;
} _guihckGuileContext;

typedef struct _functionDefinition
{
  const char* name;
  int req;
  int opt;
  int rst;
  scm_t_subr func;
} _functionDefinition;

/* Thread-local storage for guile context */
static _GUIHCK_TLS _guihckGuileContext threadLocalContext = {NULL, 0};

static void* initGuile(void*);
static void* registerFunction(void*);
static void* runStringInGuile(void* data);
static void* runExpressionInGuile(void* data);
static SCM guilePushNewElement(SCM typeSymbol);
static SCM guilePushElement(SCM elementSymbol);
static SCM guilePushElementById(SCM idSymbol);
static SCM guilePushParentElement();
static SCM guilePushChildElement(SCM childIndex);
static SCM guileSetElementProperty(SCM keySymbol, SCM value);
static SCM guileAddPropertyListener(SCM element, SCM keySymbol, SCM callback);
static SCM guileRemovePropertyListener(SCM listener);
static SCM guileGetElement();
static SCM guileGetElementProperty(SCM keySymbol);
static SCM guileGetElementChildCount();
static SCM guilePopElement();
static SCM guileSetKeyboardFocus();
static SCM guileKeyCode(SCM keyName);
static SCM guileKeyName(SCM keyCode);

void guihckGuileInit()
{
  assert(sizeof(guihckElementId) <= sizeof(scm_t_uint64) && "guihckElementId type is larger than uint64!");
  scm_with_guile(initGuile, NULL);
}

void guihckGuileRegisterFunction(const char* name, int req, int opt, int rst, scm_t_subr func)
{
  _functionDefinition fd = {name, req, opt, rst, func};
  scm_with_guile(registerFunction, &fd);
}

SCM guihckGuileRunScript(guihckContext* ctx, const char* script)
{
  threadLocalContext.ctx = ctx;
  threadLocalContext.ctxRefs += 1;

  SCM result = scm_with_guile(runStringInGuile, &script);
#if 0
  if(result)
  {
    char* resultStr = scm_to_utf8_string(scm_object_to_string(result, SCM_UNDEFINED));
    printf("RESULT: %s\n", resultStr);
    free(resultStr);
  }
  else
  {
    printf("RESULT: NULL\n");
  }
#endif
  threadLocalContext.ctxRefs -= 1;
  if(threadLocalContext.ctxRefs <= 0)
  {
    threadLocalContext.ctx = NULL;
    threadLocalContext.ctxRefs = 0;
  }
  return result;
}


SCM guihckGuileRunExpression(guihckContext* ctx, SCM expression)
{
  threadLocalContext.ctx = ctx;
  threadLocalContext.ctxRefs += 1;

  SCM result = scm_with_guile(runExpressionInGuile, expression);
#if 0
  if(result)
  {
    char* resultStr = scm_to_utf8_string(scm_object_to_string(result, SCM_UNDEFINED));
    printf("RESULT: %s\n", resultStr);
    free(resultStr);
  }
  else
  {
    printf("RESULT: NULL\n");
  }
#endif

  threadLocalContext.ctxRefs -= 1;
  if(threadLocalContext.ctxRefs <= 0)
  {
    threadLocalContext.ctx = NULL;
    threadLocalContext.ctxRefs = 0;
  }

  return result;
}

void* initGuile(void* data)
{
  (void) data;

  scm_c_define_gsubr("push-new-element!", 1, 0, 0, guilePushNewElement);
  scm_c_define_gsubr("push-element!", 1, 0, 0, guilePushElement);
  scm_c_define_gsubr("push-element-by-id!", 1, 0, 0, guilePushElementById);
  scm_c_define_gsubr("push-parent-element!", 0, 0, 0, guilePushParentElement);
  scm_c_define_gsubr("push-child-element!", 1, 0, 0, guilePushChildElement);
  scm_c_define_gsubr("pop-element!", 0, 0, 0, guilePopElement);
  scm_c_define_gsubr("set-element-property!", 2, 0, 0, guileSetElementProperty);
  scm_c_define_gsubr("add-element-property-listener!", 3, 0, 0, guileAddPropertyListener);
  scm_c_define_gsubr("remove-element-property-listener!", 1, 0, 0, guileRemovePropertyListener);
  scm_c_define_gsubr("get-element", 0, 0, 0, guileGetElement);
  scm_c_define_gsubr("get-element-property", 1, 0, 0, guileGetElementProperty);
  scm_c_define_gsubr("get-element-child-count", 0, 0, 0, guileGetElementChildCount);
  scm_c_define_gsubr("keyboard-focus!", 0, 0, 0, guileSetKeyboardFocus);
  scm_c_define_gsubr("keyboard", 1, 0, 0, guileKeyCode);
  scm_c_define_gsubr("keyboard-name", 1, 0, 0, guileKeyName);

  scm_c_eval_string(GUIHCK_GUILE_DEFAULT_SCM);

  return NULL;
}

void* registerFunction(void* data)
{
  _functionDefinition* fd = data;
  return scm_c_define_gsubr(fd->name, fd->req, fd->opt, fd->rst, fd->func);
}

void* runStringInGuile(void* data)
{
  return scm_c_eval_string((*(const char**) data));
}

void* runExpressionInGuile(void* data)
{
  SCM expression = data;
#if 0
  char* expressionStr = scm_to_utf8_string(scm_object_to_string(expression, SCM_UNDEFINED));
  printf("EXPR: %s\n", expressionStr);
  free(expressionStr);
#endif
  SCM result = scm_primitive_eval(expression);
  if(!result)
    exit(1);
  return result;
}

SCM guilePushNewElement(SCM typeSymbol)
{
  if(scm_is_symbol(typeSymbol))
  {
    char* typeName = scm_to_utf8_string(scm_symbol_to_string(typeSymbol));
    guihckStackPushNewElement(threadLocalContext.ctx, typeName);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}

SCM guilePushElement(SCM elementSymbol)
{
  if(scm_is_integer(elementSymbol))
  {
    guihckElementId id = scm_to_uint64(elementSymbol);
    guihckStackPushElement(threadLocalContext.ctx, id);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}
SCM guilePushElementById(SCM idSymbol)
{
  if(scm_is_symbol(idSymbol))
  {
    char* id = scm_to_utf8_string(scm_symbol_to_string(idSymbol));
    guihckStackPushElementById(threadLocalContext.ctx, id);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}
SCM guilePushParentElement()
{
  guihckStackPushParentElement(threadLocalContext.ctx);
  return SCM_BOOL_T;
}

SCM guilePushChildElement(SCM childIndex)
{
  if(scm_is_integer(childIndex))
  {
    guihckStackPushChildElement(threadLocalContext.ctx, scm_to_int(childIndex));
    return SCM_BOOL_T;
  }
  return SCM_BOOL_F;
}

SCM guileSetElementProperty(SCM keySymbol, SCM value)
{
  if(scm_is_symbol(keySymbol))
  {
    char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
    guihckStackElementProperty(threadLocalContext.ctx, key, value);
    free(key);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}

static void guilePropertyListenerCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) listenedId;
  (void) property;

  guihckStackPushElement(ctx, listenerId);
  SCM callback = data;
  guihckGuileRunExpression(ctx, scm_list_2(callback, value));
  guihckStackPopElement(ctx);

}

static void guilePropertyListenerFreeCallback(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  (void) ctx;
  (void) listenerId;
  (void) listenedId;
  (void) property;
  (void) value;

  SCM callback = data;
  scm_gc_unprotect_object(callback);
}

static SCM guileAddPropertyListener(SCM element, SCM keySymbol, SCM callback)
{
  char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
  guihckElementId listenerId = guihckStackGetElement(threadLocalContext.ctx);
  guihckPropertyListenerId id = guihckElementAddListener(threadLocalContext.ctx, listenerId, scm_to_uint64(element), key,
                                                         guilePropertyListenerCallback, callback, guilePropertyListenerFreeCallback);
  scm_gc_protect_object(callback);
  free(key);
  return scm_from_uint64(id);
}

static SCM guileRemovePropertyListener(SCM listener)
{
  guihckElementRemoveListener(threadLocalContext.ctx, scm_to_uint64(listener));
  return SCM_BOOL_T;
}

SCM guileGetElement()
{
  guihckElementId id = guihckStackGetElement(threadLocalContext.ctx);
  return scm_from_uint64(id);
}

SCM guileGetElementProperty(SCM keySymbol)
{
  if(scm_is_symbol(keySymbol))
  {
    char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
    return guihckStackGetElementProperty(threadLocalContext.ctx, key);
  }
  else
  {
    return SCM_UNDEFINED;
  }
}

SCM guileGetElementChildCount()
{
  return scm_from_int32(guihckStackGetElementChildCount(threadLocalContext.ctx));
}

SCM guilePopElement()
{
  guihckStackPopElement(threadLocalContext.ctx);
  return SCM_BOOL_T;
}

SCM guileSetKeyboardFocus()
{
  guihckContextKeyboardFocus(threadLocalContext.ctx, guihckStackGetElement(threadLocalContext.ctx));
  return SCM_BOOL_T;
}

SCM guileKeyCode(SCM keyName)
{
  char* keyNameStr;
  if(scm_is_symbol(keyName))
    keyNameStr = scm_to_utf8_string(scm_symbol_to_string(keyName));
  else if(scm_is_string(keyName))
    keyNameStr = scm_to_utf8_string(keyName);
  else
    assert(false && "Key name must be a symbol or a string");
  guihckKey keyCode = guihckContextGetKeyCode(threadLocalContext.ctx, keyNameStr);
  free(keyNameStr);

  return scm_from_int32(keyCode);
}

SCM guileKeyName(SCM keyCode)
{
  guihckKey code = scm_to_int32(keyCode);
  const char* keyName = guihckContextGetKeyName(threadLocalContext.ctx, code);
  return keyName ? scm_from_utf8_string(keyName) : SCM_UNDEFINED;
}

