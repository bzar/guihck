#include "guihckGuile.h"
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
} _guihckGuileContext;

/* Thread-local storage for guile context */
static _GUIHCK_TLS _guihckGuileContext threadLocalContext;

static SCM guileCreateElement(SCM typeSymbol)
{
  if(scm_symbol_p(typeSymbol))
  {
    char* typeName = scm_to_utf8_string(scm_symbol_to_string(typeSymbol));
    guihckContextCreateElement(threadLocalContext.ctx, typeName);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}

static SCM guilePushElement(SCM idSymbol)
{
  if(scm_symbol_p(idSymbol))
  {
    char* id = scm_to_utf8_string(scm_symbol_to_string(idSymbol));
    guihckContextPushElementById(threadLocalContext.ctx, id);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}
static SCM guileSetElementProperty(SCM keySymbol, SCM value)
{
  if(scm_symbol_p(keySymbol))
  {
    char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
    guihckContextElementProperty(threadLocalContext.ctx, key, value);
    return SCM_BOOL_T;
  }
  else
  {
    return SCM_BOOL_F;
  }
}

static SCM guileGetElementProperty(SCM keySymbol)
{
  if(scm_symbol_p(keySymbol))
  {
    char* key = scm_to_utf8_string(scm_symbol_to_string(keySymbol));
    return guihckContextGetElementProperty(threadLocalContext.ctx, key);
  }
  else
  {
    return SCM_UNDEFINED;
  }
}

static SCM guilePopElement()
{
  guihckContextPopElement(threadLocalContext.ctx);
  return SCM_BOOL_T;
}

static void registerApi()
{
  scm_c_define_gsubr("create-element!", 1, 0, 0, guileCreateElement);
  scm_c_define_gsubr("push-element!", 1, 0, 0, guilePushElement);
  scm_c_define_gsubr("pop-element!", 0, 0, 0, guilePopElement);
  scm_c_define_gsubr("set-element-property!", 2, 0, 0, guileSetElementProperty);
  scm_c_define_gsubr("get-element-property!", 1, 0, 0, guileGetElementProperty);
}

static void* runStringInGuile(void* data)
{
  registerApi();
  return scm_c_eval_string((*(const char**) data));
}

static void* runExpressionInGuile(void* data)
{
  SCM expression = data;
  scm_gc_protect_object(expression);
  SCM result = scm_primitive_eval(expression);
  scm_gc_unprotect_object(expression);
  return result;
}

SCM guihckContextRunGuile(guihckContext* ctx, const char* script)
{
  threadLocalContext.ctx = ctx;
  SCM result = scm_with_guile(runStringInGuile, &script);
  threadLocalContext.ctx = NULL;
  return result;
}


SCM guihckContextRunGuileExpression(guihckContext* ctx, SCM expression)
{
  threadLocalContext.ctx = ctx;
  SCM result = scm_with_guile(runExpressionInGuile, expression);
  threadLocalContext.ctx = NULL;
  return result;
}
