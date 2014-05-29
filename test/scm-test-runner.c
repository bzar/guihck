#include "guihck.h"
#include "guihckElements.h"

#include <stdio.h>

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  if(argc < 2)
  {
    printf("Usage: scm-test-runner <scm file>\n");
    return EXIT_FAILURE;
  }

  guihckInit();
  guihckContext* ctx = guihckContextNew();
  guihckElementsAddItemType(ctx);

  bool success = guihckContextExecuteScriptFile(ctx, argv[1]);

  guihckContextFree(ctx);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
