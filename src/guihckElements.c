#include "guihckElements.h"
#include <stdio.h>

static const char GUIHCK_ITEM_SCM[] =
    "(define (item props . children)"
    "  (create-element 'item (append '(x 0 y 0) props) children))";
static const char GUIHCK_MOUSEAREA_SCM[] =
    "(define (mouse-area props . children)"
    "  (create-element 'mouse-area (append '(x 0 y 0 width 0 height 0) props) children))";
static const char GUIHCK_ROW_SCM[] =
 "(define row-align-children"
 "  '(let ((x 0) (spacing (get-element-property 'spacing)))"
 "    (with-children (lambda ()"
 "      (begin"
 "        (set-element-property! 'x x)"
 "        (set! x (+ x spacing (get-element-property 'width))))))))"

 "(define row"
 "  (composite item (list 'spacing 0"
 "                        'xChanged row-align-children"
 "                        'onLoaded row-align-children)))";


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
  guihckElementsAddItemType(ctx);
  guihckElementsAddMouseAreaType(ctx);
  guihckElementsAddRowType(ctx);
}

void guihckElementsAddItemType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = { NULL, NULL, NULL, NULL };
  guihckElementTypeAdd(ctx, "item", functionMap, 0);
  guihckContextExecuteScript(ctx, GUIHCK_ITEM_SCM);
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

void guihckElementsAddRowType(guihckContext* ctx)
{
  guihckContextExecuteScript(ctx, GUIHCK_ROW_SCM);
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
  guihckElementUpdateAbsoluteCoordinates(ctx, id);
  SCM x = guihckElementGetProperty(ctx, id, "absolute-x");
  SCM y = guihckElementGetProperty(ctx, id, "absolute-y");
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
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}

bool mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseUp");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}

bool mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseMove");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}
bool mouseAreaMouseEnter(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseEnter");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}
bool mouseAreaMouseExit(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  SCM handler = guihckElementGetProperty(ctx, id, "onMouseExit");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}
