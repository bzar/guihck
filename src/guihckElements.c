#include "guihckElements.h"
#include "guihckElementUtils.h"

#include <stdio.h>

static const char GUIHCK_ITEM_SCM[] =
    "(define (item . args)"
    "  (define default-args (list (prop 'x 0) (prop 'y 0)))"
    "  (create-element 'item (append default-args args)))";
static const char GUIHCK_MOUSEAREA_SCM[] =
    "(define (mouse-area . args)"
    "  (define default-args (list (prop 'x 0) (prop 'y 0) (prop 'width 0) (prop 'height 0)"
    "                             (prop 'hover #f) (prop 'pressed #f)))"
    "  (create-element 'mouse-area (append default-args args)))";
static const char GUIHCK_ROW_SCM[] =
    "(define (row-gen)"
    "  (define (align-width w)"
    "    (let ((x 0) (spacing (get-prop 'spacing)))"
    "      (for-each"
    "        (lambda (child)"
    "          (set-prop! child 'x x)"
    "          (set! x (+ x spacing (get-prop child 'width))))"
    "        (get-prop 'children))"
    "      (set-prop! 'width (- x spacing))))"

    "  (define (align-height h) "
    "    (set-prop! 'height"
    "      (apply max "
    "        (cons 0 (map "
    "          (lambda (c) (get-prop c 'height)) "
    "          (get-prop 'children))))))"

    "  (define (update-bindings cs)"
    "    (let ((previous (get-prop 'listeners)))"
    "      (if (pair? previous)"
    "        (for-each unbind previous)))"
    "    (align-width 0)"
    "    (align-height 0)"
    "    (flatmap"
    "      (lambda (c) "
    "        (list "
    "          (bind c 'width align-width)"
    "          (bind c 'height align-height)))"
    "      cs))"

    "  (define default-args (list"
    "    (prop 'spacing 0)"
    "    (prop 'init (lambda ()"
    "      (bind 'spacing align-width)"
    "      (align-width 0)))"
    "    (prop 'listeners"
    "      (bound '(this children) update-bindings))))"

    "  (apply composite (cons item default-args)))"
    "(define row (row-gen))"
    ;

static const char GUIHCK_COLUMN_SCM[] =
    "(define (column-gen)"
    "  (define (align-height h)"
    "    (let ((y 0) (spacing (get-prop 'spacing)))"
    "      (for-each"
    "        (lambda (child)"
    "          (set-prop! child 'y y)"
    "          (set! y (+ y spacing (get-prop child 'height))))"
    "        (get-prop 'children))"
    "      (set-prop! 'height (- y spacing))))"

    "  (define (align-width w) "
    "    (set-prop! 'width"
    "      (apply max "
    "        (cons 0 (map "
    "          (lambda (c) (get-prop c 'width)) "
    "          (get-prop 'children))))))"

    "  (define (update-bindings cs)"
    "    (let ((previous (get-prop 'listeners)))"
    "      (if (pair? previous)"
    "        (for-each unbind previous)))"
    "    (align-width 0)"
    "    (align-height 0)"
    "    (flatmap"
    "      (lambda (c) "
    "        (list "
    "          (bind c 'width align-width)"
    "          (bind c 'height align-height)))"
    "      cs))"

    "  (define default-args (list"
    "    (prop 'spacing 0)"
    "    (prop 'init (lambda ()"
    "      (bind 'spacing align-height)"
    "      (align-height 0)))"
    "    (prop 'listeners"
    "      (bound '(this children) update-bindings))))"

    "  (apply composite (cons item default-args)))"
    "(define column (column-gen))"
    ;

static const char GUIHCK_TIMER_SCM[] =
    "(define (timer . args)"
    "  (define default-args (list (prop 'running #f) (prop 'next-timeout -1) (prop 'interval 0) (prop 'repeat 1) (prop 'on-timeout (lambda () #f))))"
    "  (create-element 'timer (append default-args args)))";

static void initItem(guihckContext* ctx, guihckElementId id, void* data);

static void initMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static bool updateMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static bool mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static bool mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static bool mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
static bool mouseAreaMouseEnter(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
static bool mouseAreaMouseExit(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);

static void initTimer(guihckContext* ctx, guihckElementId id, void* data);
static bool updateTimer(guihckContext* ctx, guihckElementId id, void* data);

void guihckElementsAddAllTypes(guihckContext* ctx)
{
  guihckElementsAddItemType(ctx);
  guihckElementsAddMouseAreaType(ctx);
  guihckElementsAddRowType(ctx);
  guihckElementsAddColumnType(ctx);
  guihckElementsAddTimerType(ctx);
}

void guihckElementsAddItemType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = { initItem, NULL, NULL, NULL, NULL, NULL };
  guihckElementTypeAdd(ctx, "item", functionMap, 0);
  guihckContextExecuteScript(ctx, GUIHCK_ITEM_SCM);
}

void guihckElementsAddMouseAreaType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initMouseArea,
    destroyMouseArea,
    updateMouseArea,
    NULL,
    NULL,
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

void guihckElementsAddTimerType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initTimer,
    NULL,
    updateTimer,
    NULL,
    NULL,
    NULL
  };
  guihckElementTypeAdd(ctx, "timer", functionMap, 0);
  guihckContextExecuteScript(ctx, GUIHCK_TIMER_SCM);
}

void initItem(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) data;

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
  guihckElementAddUpdateProperty(ctx, id, "absolute-x");
  guihckElementAddUpdateProperty(ctx, id, "absolute-y");
  guihckElementAddUpdateProperty(ctx, id, "width");
  guihckElementAddUpdateProperty(ctx, id, "height");
}

void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) id;

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
  (void) data;
  (void) button;
  (void) x;
  (void) y;

  bool handled = false;
  guihckElementProperty(ctx, id, "pressed", SCM_BOOL_T);
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-down");
  if(scm_to_bool(scm_procedure_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   SCM expression = scm_list_4(handler, scm_from_int8(button), scm_from_double(x), scm_from_double(y));
   SCM result = guihckContextExecuteExpression(ctx, expression);
   handled = scm_is_eq(result, SCM_BOOL_T);
   guihckStackPopElement(ctx);
  }
  return handled;
}

bool mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  (void) data;
  (void) button;
  (void) x;
  (void) y;

  bool handled = false;
  SCM pressed = guihckElementGetProperty(ctx, id, "pressed");
  bool clicked = scm_to_bool(pressed);
  guihckElementProperty(ctx, id, "pressed", SCM_BOOL_F);

  {
    SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-up");
    if(scm_to_bool(scm_procedure_p(handler)))
    {
      guihckStackPushElement(ctx, id);
      SCM expression = scm_list_4(handler, scm_from_int8(button), scm_from_double(x), scm_from_double(y));
      SCM result = guihckContextExecuteExpression(ctx, expression);
      handled = scm_is_eq(result, SCM_BOOL_T);
      guihckStackPopElement(ctx);
    }
  }

  if(clicked && !handled)
  {
    SCM handler = guihckElementGetProperty(ctx, id, "on-click");
    if(scm_to_bool(scm_procedure_p(handler)))
    {
      guihckStackPushElement(ctx, id);
      SCM expression = scm_list_4(handler, scm_from_int8(button), scm_from_double(x), scm_from_double(y));
      SCM result = guihckContextExecuteExpression(ctx, expression);
      handled = scm_is_eq(result, SCM_BOOL_T);
      guihckStackPopElement(ctx);
    }
  }

  return handled;
}

bool mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  (void) data;
  (void) sx;
  (void) sy;
  (void) dx;
  (void) dy;

  bool handled = false;
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-move");
  if(scm_to_bool(scm_procedure_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   SCM expression = scm_list_5(handler, scm_from_double(sx), scm_from_double(sy), scm_from_double(dx), scm_from_double(dy));
   SCM result = guihckContextExecuteExpression(ctx, expression);
   handled = scm_is_eq(result, SCM_BOOL_T);
   guihckStackPopElement(ctx);
  }
  return handled;
}
bool mouseAreaMouseEnter(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  (void) data;
  (void) sx;
  (void) sy;
  (void) dx;
  (void) dy;

  bool handled = false;
  guihckElementProperty(ctx, id, "hover", SCM_BOOL_T);
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-enter");
  if(scm_to_bool(scm_procedure_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   SCM expression = scm_list_5(handler, scm_from_double(sx), scm_from_double(sy), scm_from_double(dx), scm_from_double(dy));
   SCM result = guihckContextExecuteExpression(ctx, expression);
   handled = scm_is_eq(result, SCM_BOOL_T);
   guihckStackPopElement(ctx);
  }
  return handled;
}
bool mouseAreaMouseExit(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  (void) data;
  (void) sx;
  (void) sy;
  (void) dx;
  (void) dy;

  bool handled = false;
  guihckElementProperty(ctx, id, "hover", SCM_BOOL_F);
  guihckElementProperty(ctx, id, "pressed", SCM_BOOL_F);
  SCM handler = guihckElementGetProperty(ctx, id, "on-mouse-exit");
  if(scm_to_bool(scm_procedure_p(handler)))
  {
   guihckStackPushElement(ctx, id);
   SCM expression = scm_list_5(handler, scm_from_double(sx), scm_from_double(sy), scm_from_double(dx), scm_from_double(dy));
   SCM result = guihckContextExecuteExpression(ctx, expression);
   handled = scm_is_eq(result, SCM_BOOL_T);
   guihckStackPopElement(ctx);
  }
  return handled;
}

void initTimer(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) data;

  guihckElementAddUpdateProperty(ctx, id, "running");
}

bool updateTimer(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) data;

  bool running = scm_is_true(guihckElementGetProperty(ctx, id, "running"));
  if(running)
  {
    double nextTimeout = scm_to_double(guihckElementGetProperty(ctx, id, "next-timeout"));
    double current = guihckContextGetTime(ctx);

    if(nextTimeout < 0)
    {
      double interval = scm_to_double(guihckElementGetProperty(ctx, id, "interval"));
      guihckElementProperty(ctx, id, "next-timeout", scm_from_double(current + interval));
      guihckElementProperty(ctx, id, "cycle", scm_from_int32(0));
    }
    else if(current >= nextTimeout)
    {
      int repeat = scm_to_int32(guihckElementGetProperty(ctx, id, "repeat"));
      int cycle = scm_to_int32(guihckElementGetProperty(ctx, id, "cycle"));
      cycle += 1;

      if(repeat < 0 || repeat > cycle)
      {
        double interval = scm_to_double(guihckElementGetProperty(ctx, id, "interval"));
        guihckElementProperty(ctx, id, "next-timeout", scm_from_double(nextTimeout + interval));
      }
      else
      {
        running = false;
        guihckElementProperty(ctx, id, "next-timeout", scm_from_double(-1));
        guihckElementProperty(ctx, id, "running", SCM_BOOL_F);
      }

      guihckElementProperty(ctx, id, "cycle", scm_from_int32(cycle));
      SCM onTimeout = guihckElementGetProperty(ctx, id, "on-timeout");
      guihckStackPushElement(ctx, id);
      SCM expression = scm_list_2(onTimeout, scm_from_int32(cycle));
      guihckContextExecuteExpression(ctx, expression);
      guihckStackPopElement(ctx);
    }
  }
  return running;
}
