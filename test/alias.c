#include "guihck.h"
#include "guihckElements.h"

#include <stdio.h>

const char TEST_SCM[] =

    "(import (rnrs (6)))"

    "(create-elements!"
    "  (item '(id item-1 value 0))"
    "  (item '(id item-2 value (alias (find-element 'item-1) 'value)))"
    "  (item '(id item-3 value (alias (find-element 'item-2) 'value))))"

    "(define (display-all . things) (for-each display things))"

    "(define (test id value)"
    "  (begin"
    "    (display-all id \": \" (get-prop (find-element id) 'value) \" = \" value \"\n\")"
    "    (assert (= (get-prop (find-element id) 'value) value))))"

    "(test 'item-1 0)"
    "(test 'item-2 0)"
    "(test 'item-3 0)"

    "(set-prop! (find-element 'item-1) 'value 1)"

    "(test 'item-1 1)"
    "(test 'item-2 1)"
    "(test 'item-3 1)"

    "(set-prop! (find-element 'item-2) 'value 2)"

    "(test 'item-1 2)"
    "(test 'item-2 2)"
    "(test 'item-3 2)"

    "(set-prop! (find-element 'item-3) 'value 3)"

    "(test 'item-1 3)"
    "(test 'item-2 3)"
    "(test 'item-3 3)"
;

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  guihckInit();
  guihckContext* ctx = guihckContextNew();
  guihckElementsAddItemType(ctx);
  guihckContextExecuteScript(ctx, TEST_SCM);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}