#include "guihckGuile.h"

typedef struct _guihckGuileContext
{
  guihckGui* gui;
  const char* script;
} _guihckGuileContext;


static _guihckGuileContext* globalContext;

static SCM guileCreateElement(SCM typeSymbol)
{
  if(scm_symbol_p(typeSymbol))
  {
    char* typeName = scm_to_utf8_string(scm_symbol_to_string(typeSymbol));
    guihckGuiCreateElement(globalContext->gui, typeName);
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
    guihckGuiElementProperty(globalContext->gui, key, value);
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
    return guihckGuiGetElementProperty(globalContext->gui, key);
  }
  else
  {
    return SCM_UNDEFINED;
  }
}

static SCM guilePopElement()
{
  guihckGuiPopElement(globalContext->gui);
  return SCM_BOOL_T;
}

static void* runInGuile(void* data)
{
  _guihckGuileContext* ctx = data;
  globalContext = ctx;
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
