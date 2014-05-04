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
  guihckGui* gui;
  const char* script;
} _guihckGuileContext;

/* Thread-local storage for guile context */
static _GUIHCK_TLS _guihckGuileContext* threadLocalContext;

static SCM guileCreateElement(SCM typeSymbol)
{
  if(scm_symbol_p(typeSymbol))
  {
    char* typeName = scm_to_utf8_string(scm_symbol_to_string(typeSymbol));
    guihckGuiCreateElement(threadLocalContext->gui, typeName);
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
    guihckGuiElementProperty(threadLocalContext->gui, key, value);
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
    return guihckGuiGetElementProperty(threadLocalContext->gui, key);
  }
  else
  {
    return SCM_UNDEFINED;
  }
}

static SCM guilePopElement()
{
  guihckGuiPopElement(threadLocalContext->gui);
  return SCM_BOOL_T;
}

static void* runInGuile(void* data)
{
  _guihckGuileContext* ctx = data;
  threadLocalContext = ctx;
  scm_c_define_gsubr("create-element!", 1, 0, 0, guileCreateElement);
  scm_c_define_gsubr("pop-element!", 0, 0, 0, guilePopElement);
  scm_c_define_gsubr("set-element-property!", 2, 0, 0, guileSetElementProperty);
  scm_c_define_gsubr("get-element-property!", 1, 0, 0, guileGetElementProperty);
  SCM exp = scm_from_utf8_string(ctx->script);
  SCM result = scm_eval_string(exp);
  return result;
}

SCM guihckGuiRunGuile(guihckGui* gui, const char* script)
{
  _guihckGuileContext ctx;
  ctx.gui = gui;
  ctx.script = script;
  return (SCM) scm_with_guile(runInGuile, &ctx);
}
