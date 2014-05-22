#include "guihckElements.h"
#include <stdio.h>

static const char GUIHCK_ITEM_SCM[] =
    "(define (item props . children)"
    "  (create-element 'item (append '(x 0 y 0) props) children))";
static const char GUIHCK_MOUSEAREA_SCM[] =
    "(define (mouse-area props . children)"
    "  (create-element 'mouse-area (append '(x 0 y 0 width 0 height 0) props) children))";
static const char GUIHCK_ROW_SCM[] =
    "(define (row-align-width w)"
    "  (let ((x 0) (spacing (get-prop 'spacing)))"
    "    (for-each"
    "      (lambda (child)"
    "        (set-prop! child 'x x)"
    "        (set! x (+ x spacing (get-prop child 'width))))"
    "      (get-prop 'children))"
    "    (set-prop! 'width (- x spacing))))"

    "(define (row-align-height h) "
    "  (set-prop! 'height"
    "    (apply max "
    "      (cons 0 (map "
    "        (lambda (c) (get-prop c 'height)) "
    "        (get-prop 'children))))))"

    "(define (row-update-bindings cs)"
    "  (let ((previous (get-prop 'listeners)))"
    "    (if (pair? previous)"
    "      (for-each unbind previous)))"
    "  (row-align-width 0)"
    "  (row-align-height 0)"
    "  (flatmap"
    "    (lambda (c) "
    "      (list "
    "        (bind c 'width row-align-width)"
    "        (bind c 'height row-align-height)))"
    "    cs))"

    "(define row"
    "  (composite item (list 'spacing 0"
    "                        'init '(begin"
    "                          (bind 'spacing row-align-width)"
    "                          (row-align-width 0))"
    "                        'listeners '(bind (observe 'this 'children) row-update-bindings))))"
    ;

static const char GUIHCK_COLUMN_SCM[] =
    "(define (column-align-height h)"
    "  (let ((y 0) (spacing (get-prop 'spacing)))"
    "    (for-each"
    "      (lambda (child)"
    "        (set-prop! child 'y y)"
    "        (set! y (+ y spacing (get-prop child 'height))))"
    "      (get-prop 'children))"
    "    (set-prop! 'height (- y spacing))))"

    "(define (column-align-width w) "
    "  (set-prop! 'width"
    "    (apply max "
    "      (cons 0 (map "
    "        (lambda (c) (get-prop c 'width)) "
    "        (get-prop 'children))))))"

    "(define (column-update-bindings cs)"
    "  (let ((previous (get-prop 'listeners)))"
    "    (if (pair? previous)"
    "      (for-each unbind previous)))"
    "  (column-align-width 0)"
    "  (column-align-height 0)"
    "  (flatmap"
    "    (lambda (c) "
    "      (list "
    "        (bind c 'width column-align-width)"
    "        (bind c 'height column-align-height)))"
    "    cs))"

    "(define column"
    "  (composite item (list 'spacing 0"
    "                        'init '(begin"
    "                          (bind 'spacing column-align-height)"
    "                          (column-align-height 0))"
    "                        'listeners '(bind (observe 'this 'children) column-update-bindings))))"
    ;


static void initItem(guihckContext* ctx, guihckElementId id, void* data);
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
  guihckElementsAddColumnType(ctx);
}

void guihckElementsAddItemType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = { initItem, NULL, NULL, NULL };
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
void guihckElementsAddColumnType(guihckContext* ctx)
{
  guihckContextExecuteScript(ctx, GUIHCK_COLUMN_SCM);
}

void initItem(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckElementAddParentPositionListeners(ctx, id);
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
  guihckElementAddParentPositionListeners(ctx, id);
}

void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckMouseAreaRemove(ctx, *((guihckMouseAreaId*) data));
}

bool updateMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
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

  guihckMouseAreaRect(ctx, *((guihckMouseAreaId*) data), px, py, pw, ph);

  return false;
}
bool mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-down");
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
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-up");
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
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-move");
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
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-enter");
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
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-exit");
  if(scm_to_bool(scm_list_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   guihckContextExecuteExpression(ctx, handler);
   guihckStackPopElement(ctx);
  }
  return false;
}
