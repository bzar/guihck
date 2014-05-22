#include "guihckElementUtils.h"



void guihckElementUpdateAbsoluteCoordinates(guihckContext* ctx, guihckElementId elementId)
{
  float x = 0;
  float y = 0;
  guihckElementId id = elementId;
  guihckElementId rootId = guihckContextGetRootElement(ctx);
  do
  {
    SCM xProperty = guihckElementGetProperty(ctx, id, "x");
    SCM yProperty = guihckElementGetProperty(ctx, id, "y");
    x += xProperty && scm_is_real(xProperty) ? scm_to_double(xProperty) : 0;
    y += yProperty && scm_is_real(yProperty) ? scm_to_double(yProperty) : 0;
    id = guihckElementGetParent(ctx, id);
  }
  while(id != rootId);

  guihckElementProperty(ctx, elementId, "absolute-x", scm_from_double(x));
  guihckElementProperty(ctx, elementId, "absolute-y", scm_from_double(y));
}

static void updateAbsoluteX(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  guihckElementId parent = guihckElementGetParent(ctx, listenerId);
  SCM x = guihckElementGetProperty(ctx, listenerId, "x");
  SCM pax = guihckElementGetProperty(ctx, parent, "absolute-x");
  SCM ax =
      scm_is_real(pax) && scm_is_real(x) ? scm_sum(x, pax) :
      scm_is_real(x) ? x :
      scm_is_real(pax) ? pax :
      scm_from_double(0);

  guihckElementProperty(ctx, listenerId, "absolute-x", ax);
}
static void updateAbsoluteY(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data)
{
  guihckElementId parent = guihckElementGetParent(ctx, listenerId);
  SCM y = guihckElementGetProperty(ctx, listenerId, "y");
  SCM pay = guihckElementGetProperty(ctx, parent, "absolute-y");
  SCM ay =
      scm_is_real(pay) && scm_is_real(y) ? scm_sum(y, pay) :
      scm_is_real(y) ? y :
      scm_is_real(pay) ? pay :
      scm_from_double(0);

  guihckElementProperty(ctx, listenerId, "absolute-y", ay);
}

void guihckElementAddParentPositionListeners(guihckContext* ctx, guihckElementId id)
{
  guihckElementId parent = guihckElementGetParent(ctx, id);
  guihckElementAddListener(ctx, id, id, "x", updateAbsoluteX, NULL, NULL);
  guihckElementAddListener(ctx, id, parent, "absolute-x", updateAbsoluteX, NULL, NULL);
  guihckElementAddListener(ctx, id, id, "y", updateAbsoluteY, NULL, NULL);
  guihckElementAddListener(ctx, id, parent, "absolute-y", updateAbsoluteY, NULL, NULL);
}
