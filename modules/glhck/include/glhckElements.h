#ifndef GUIHCK_GLHCK_ELEMENTS_H
#define GUIHCK_GLHCK_ELEMENTS_H

#include "guihck.h"

#ifdef __cplusplus
extern "C" {
#endif

void guihckGlhckAddAllTypes(guihckContext* ctx);

void guihckGlhckAddRectangleType(guihckContext* ctx);
void guihckGlhckAddTextType(guihckContext* ctx);
void guihckGlhckAddImageType(guihckContext* ctx);
void guihckGlhckAddTextInputType(guihckContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
