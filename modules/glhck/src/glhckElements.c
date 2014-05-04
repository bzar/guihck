#include "glhckElements.h"
#include "glhck/glhck.h"
#include <stdio.h>

static void initRectangle(guihckContext* ctx, guihckElementId id, void* data);
static void destroyRectangle(guihckContext* ctx, guihckElementId id, void* data);
static char updateRectangle(guihckContext* ctx, guihckElementId id, void* data);
static void renderRectangle(guihckContext* ctx, guihckElementId id, void* data);

void guihckGlhckAddAllTypes(guihckContext* ctx)
{
  guihckGlhckAddRectangleType(ctx);
}

void guihckGlhckAddRectangleType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initRectangle,
    destroyRectangle,
    updateRectangle,
    renderRectangle
  };
  guihckElementTypeAdd(ctx, "rectangle", functionMap, sizeof(glhckObject*));
}

void initRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObject* o = glhckPlaneNew(1.0, 1.0);
  glhckMaterial* m = glhckMaterialNew(NULL);
  glhckObjectMaterial(o, m);
  glhckMaterialFree(m);

/*  guihckElementId parentId = guihckElementGetParent(ctx, id);
  if(parentId != GUIHCK_NO_PARENT)
  {
    glhckObject* parent = *((glhckObject**) guihckElementGetData(ctx, parentId));
    glhckObjectAddChild(parent, o);
  }*/
  *((glhckObject**) data) = o;
}

void destroyRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObjectFree(*((glhckObject**) data));
}

char updateRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObject* o = *((glhckObject**) data);
  SCM x = guihckElementGetProperty(ctx, id, "x");
  SCM y = guihckElementGetProperty(ctx, id, "y");
  SCM w = guihckElementGetProperty(ctx, id, "width");
  SCM h = guihckElementGetProperty(ctx, id, "height");
  SCM c = guihckElementGetProperty(ctx, id, "color");

  kmVec3 position = *glhckObjectGetPosition(o);
  kmVec3 scale = *glhckObjectGetScale(o);
  if(scm_to_bool(scm_real_p(w))) scale.x = scm_to_double(w)/2;
  if(scm_to_bool(scm_real_p(h))) scale.y = scm_to_double(h)/2;
  if(scm_to_bool(scm_real_p(x))) position.x = scm_to_double(x) + scale.x;
  if(scm_to_bool(scm_real_p(y))) position.y = scm_to_double(y) + scale.y;

  glhckObjectPosition(o, &position);
  glhckObjectScale(o, &scale);

  if(scm_to_bool(scm_list_p(c)) && scm_to_int32(scm_length(c)) == 3)
  {
    glhckColorb color = *glhckMaterialGetDiffuse(glhckObjectGetMaterial(o));
    SCM r = scm_list_ref(c, scm_from_int8(0));
    SCM g = scm_list_ref(c, scm_from_int8(1));
    SCM b = scm_list_ref(c, scm_from_int8(2));

    if(scm_to_bool(scm_integer_p(r))) color.r = scm_to_uint8(r);
    if(scm_to_bool(scm_integer_p(g))) color.g = scm_to_uint8(g);
    if(scm_to_bool(scm_integer_p(b))) color.b = scm_to_uint8(b);

    glhckMaterialDiffuse(glhckObjectGetMaterial(o), &color);
  }

  return 0;
}

void renderRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObjectDraw(*((glhckObject**) data));
}
