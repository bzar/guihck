#ifndef GUIHCK_GUILE_H
#define GUIHCK_GUILE_H

#include "guihck.h"

void guihckGuileInit();
void guihckGuileRegisterFunction(const char* name, int req, int opt, int rst, scm_t_subr func);
SCM guihckGuileRunExpression(guihckContext* ctx, SCM expression);
SCM guihckGuileRunScript(guihckContext* ctx, const char* script);
guihckContext* guihckGuileGetCurrentThreadContext();

#endif
