#include "guihckElements.h"
#include <stdio.h>

static const char GUIHCK_MOUSEAREA_SCM[] =
    "(define (mouse-area props . children)"
    "  (apply create-element-with-properties (append (list 'mouse-area props) children)))";

static void initMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static bool updateMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static bool mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static bool mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static bool mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
static bool mouseAreaMouseEnter(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
static bool mouseAreaMouseExit(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);

void guihckElementsAddAllTypes(guihckContext* ctx)
{
  guihckElementsAddMouseAreaType(ctx);
}


void guihckElementsAddMouseAreaType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initMouseArea,
    destroyMouseArea,
    updateMouseArea,
    NULL
  };
  guihckElementTypeAdd(ctx, "mouse-area", functionMap, sizeof(guihckMouseAreaId));
  guihckContextExecuteScript(ctx, GUIHCK_MOUSEAREA_SCM);
}

void initMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckMouseAreaFunctionMap functionMap = {
    mouseAreaMouseDown,
    mouseAreaMouseUp,
    mouseAreaMouseMove,
    mouseAreaMouseEnter,
    mouseAreaMouseExit
  };
  *((guihckMouseAreaId*) data) = guihckMouseAreaNew(ctx, id, functionMap);
}

void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckMouseAreaRemove(ctx, *((guihckMouseAreaId*) data));
}

bool updateMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  SCM x = guihckElementGetProperty(ctx, id, "x");
  SCM y = guihckElementGetProperty(ctx, id, "y");
  SCM w = guihckElementGetProperty(ctx, id, "width");
  SCM h = guihckElementGetProperty(ctx, id, "height");

  float px, py, pw, ph;
  guihckMouseAreaGetRect(ctx, *((guihckMouseAreaId*) data), &px, &py, &pw, &ph);

  if(scm_to_bool(scm_real_p(x))) px = scm_to_double(x);
  if(scm_to_bool(scm_real_p(y))) py = scm_to_double(y);
  if(scm_to_bool(scm_real_p(w))) pw = scm_to_double(w);
  if(scm_to_bool(scm_real_p(h))) ph = scm_to_double(h);

  guihckMouseAreaRect(ctx, *((guihckMouseAreaId*) data), px, py, ph, pw);

  return false;
}
bool mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseDown");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckContextPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckContextPopElement(ctx);
  }
  return false;
}

bool mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseUp");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckContextPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckContextPopElement(ctx);
  }
  return false;
}

bool mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseMove");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckContextPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckContextPopElement(ctx);
  }
  return false;
}
bool mouseAreaMouseEnter(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseEnter");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckContextPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckContextPopElement(ctx);
  }
  return false;
}
bool mouseAreaMouseExit(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseExit");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckContextPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckContextPopElement(ctx);
  }
  return false;
}
