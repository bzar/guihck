#include "guihckElements.h"
#include <stdio.h>

static void initMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static char updateMouseArea(guihckContext* ctx, guihckElementId id, void* data);
static char mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static char mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
static char mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);

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
}

void initMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckMouseAreaFunctionMap functionMap = {
    mouseAreaMouseDown,
    mouseAreaMouseUp,
    mouseAreaMouseMove
  };
  *((guihckMouseAreaId*) data) = guihckMouseAreaNew(ctx, id, functionMap);
}

void destroyMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  guihckMouseAreaRemove(ctx, *((guihckMouseAreaId*) data));
}

char updateMouseArea(guihckContext* ctx, guihckElementId id, void* data)
{
  SCM x = guihckElementGetProperty(ctx, id, "x");
  SCM y = guihckElementGetProperty(ctx, id, "y");
  SCM w = guihckElementGetProperty(ctx, id, "width");
  SCM h = guihckElementGetProperty(ctx, id, "height");

  float px, py, pw, ph;
  guihckMouseAreaGetRect(ctx, *((guihckMouseAreaId*) data), &px, &py, &pw, &ph);
/*
  float parentX = 0;
  float parentY = 0;
  guihckElementId parentId = guihckElementGetParent(ctx, id);
  if(parentId != GUIHCK_NO_PARENT)
  {
    SCM parx =  guihckElementGetProperty(ctx, parentId, "x");
    SCM pary =  guihckElementGetProperty(ctx, parentId, "y");
    if(scm_to_bool(scm_real_p(parx))) parentX = scm_to_double(parx);
    if(scm_to_bool(scm_real_p(pary))) parentY = scm_to_double(pary);
  }
*/
  if(scm_to_bool(scm_real_p(x))) px = scm_to_double(x)/* + parentX*/;
  if(scm_to_bool(scm_real_p(y))) py = scm_to_double(y)/* + parentY*/;
  if(scm_to_bool(scm_real_p(w))) pw = scm_to_double(w);
  if(scm_to_bool(scm_real_p(h))) ph = scm_to_double(h);

  guihckMouseAreaRect(ctx, *((guihckMouseAreaId*) data), px, py, ph, pw);

  return 0;
}
char mouseAreaMouseDown(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  printf("mouseAreaMouseDown %d %d %f %f\n", id, button, x, y);
  return 0;
}

char mouseAreaMouseUp(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y)
{
  printf("mouseAreaMouseUp %d %d %f %f\n", id, button, x, y);
  return 0;
}

char mouseAreaMouseMove(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy)
{
  printf("mouseAreaMouseMove %d %f %f %f %f\n", id, sx, sy, dx, dy);
  return 0;
}
