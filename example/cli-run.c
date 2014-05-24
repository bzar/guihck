#include "guihck.h"
#include "guihckElements.h"

#include <stdio.h>

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    printf("Usage: guihck-cli-run <scm file>\n");
    return EXIT_FAILURE;
  }

  guihckInit();
  guihckContext* ctx = guihckContextNew();
  guihckElementsAddItemType(ctx);
  guihckContextExecuteScriptFile(ctx, argv[1]);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}
