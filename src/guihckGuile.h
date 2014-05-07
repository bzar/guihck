#ifndef GUIHCK_GUILE_H
#define GUIHCK_GUILE_H

#include "guihck.h"

SCM guihckContextRunGuileExpression(guihckContext* ctx, SCM expression);
SCM guihckContextRunGuile(guihckContext* ctx, const char* script);

#endif
