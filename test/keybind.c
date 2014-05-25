#include "guihck.h"
#include "guihckElements.h"

#include <stdio.h>
#include <assert.h>

const char TEST_SCM[] = ""
;

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  guihckInit();
  guihckContext* ctx = guihckContextNew();

  // Default keys name -> code
  assert(guihckContextGetKeyCode(ctx, "a") == GUIHCK_KEY_A);
  assert(guihckContextGetKeyCode(ctx, "backspace") == GUIHCK_KEY_BACKSPACE);
  assert(guihckContextGetKeyCode(ctx, "left-super") == GUIHCK_KEY_LEFT_SUPER);

  // Default keys code -> name
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_Z), "z") == 0);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_ENTER), "enter") == 0);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_KP_4), "kp-4") == 0);

  // Non-conflicting bindings
  guihckContextAddKeyBinding(ctx, GUIHCK_KEY_USER, "foo");
  guihckContextAddKeyBinding(ctx, GUIHCK_KEY_USER + 1, "bar");
  assert(guihckContextGetKeyCode(ctx, "foo") == GUIHCK_KEY_USER);
  assert(guihckContextGetKeyCode(ctx, "bar") == GUIHCK_KEY_USER + 1);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_USER), "foo") == 0);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_USER + 1), "bar") == 0);

  // Conflicting bindings
  guihckContextAddKeyBinding(ctx, GUIHCK_KEY_USER + 2, "b");
  guihckContextAddKeyBinding(ctx, GUIHCK_KEY_B, "second-b");
  assert(guihckContextGetKeyCode(ctx, "b") == GUIHCK_KEY_B);
  assert(guihckContextGetKeyCode(ctx, "second-b") == GUIHCK_KEY_B);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_USER + 2), "b") == 0);
  assert(strcmp(guihckContextGetKeyName(ctx, GUIHCK_KEY_B), "b") == 0);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}

