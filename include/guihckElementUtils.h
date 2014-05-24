#ifndef GUIHCK_ELEMENT_UTILS_H
#define GUIHCK_ELEMENT_UTILS_H

#include "guihck.h"

void guihckElementUpdateAbsoluteCoordinates(guihckContext* ctx, guihckElementId elementId);
void guihckElementAddParentPositionListeners(guihckContext* ctx, guihckElementId id);
void guihckElementAddUpdateProperty(guihckContext* ctx, guihckElementId id, const char* propertyName);

#endif
