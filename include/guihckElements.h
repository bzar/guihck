#ifndef GUIHCK_ELEMENTS_H
#define GUIHCK_ELEMENTS_H

#include "guihck.h"

#ifdef __cplusplus
extern "C" {
#endif

void guihckElementsAddAllTypes(guihckContext* ctx);

void guihckElementsAddItemType(guihckContext* ctx);
void guihckElementsAddMouseAreaType(guihckContext* ctx);
void guihckElementsAddRowType(guihckContext* ctx);
void guihckElementsAddColumnType(guihckContext* ctx);
void guihckElementsAddTimerType(guihckContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
