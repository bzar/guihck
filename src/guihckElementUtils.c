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
    x += scm_is_true(scm_real_p(xProperty)) ? scm_to_double(xProperty) : 0;
    y += scm_is_true(scm_real_p(yProperty)) ? scm_to_double(yProperty) : 0;
    id = guihckElementGetParent(ctx, id);
  }
  while(id != rootId);

  guihckElementProperty(ctx, elementId, "absolute-x", scm_from_double(x));
  guihckElementProperty(ctx, elementId, "absolute-y", scm_from_double(y));
}
