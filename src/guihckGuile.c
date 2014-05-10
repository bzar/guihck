#include "guihckGuile.h"
#include "guihckGuileDefaultScm.h"

#if defined(_MSC_VER)
# define _GUIHCK_TLS __declspec(thread)
# define _GUIHCK_TLS_FOUND
#elif defined(__GNUC__)
# define _GUIHCK_TLS __thread
# define _GUIHCK_TLS_FOUND
#else
# define _GUIHCK_TLS
# warning "No Thread-local storage! Multi-threaded guihck applications may have unexpected behaviour!"
#endif

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
static SCM guilePushElement(SCM idSymbol);
static SCM guilePushParentElement();
static SCM guilePushChildElement(SCM childIndex);
static SCM guileSetElementProperty(SCM keySymbol, SCM value);
static SCM guileGetElementProperty(SCM keySymbol);
static SCM guileGetElementChildCount();
static SCM guilePopElement();

void guihckGuileInit()
{
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
  scm_c_define_gsubr("push-new-element!", 1, 0, 0, guilePushNewElement);
  scm_c_define_gsubr("push-element!", 1, 0, 0, guilePushElement);
  scm_c_define_gsubr("push-parent-element!", 0, 0, 0, guilePushParentElement);
  scm_c_define_gsubr("push-child-element!", 1, 0, 0, guilePushChildElement);
  scm_c_define_gsubr("pop-element!", 0, 0, 0, guilePopElement);
  scm_c_define_gsubr("set-element-property!", 2, 0, 0, guileSetElementProperty);
  scm_c_define_gsubr("get-element-property", 1, 0, 0, guileGetElementProperty);
  scm_c_define_gsubr("get-element-child-count", 0, 0, 0, guileGetElementChildCount);

  scm_c_eval_string(GUIHCK_GUILE_DEFAULT_SCM);
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
  scm_gc_protect_object(expression);
  SCM result = scm_primitive_eval(expression);
  scm_gc_unprotect_object(expression);
  return result;
}

SCM guilePushNewElement(SCM typeSymbol)
{
  if(scm_symbol_p(typeSymbol))
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

SCM guilePushElement(SCM idSymbol)
{
  if(scm_symbol_p(idSymbol))
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
  if(scm_symbol_p(keySymbol))
  {
    char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
    guihckStackElementProperty(threadLocalContext.ctx, key, value);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}

SCM guileGetElementProperty(SCM keySymbol)
{
  if(scm_symbol_p(keySymbol))
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
