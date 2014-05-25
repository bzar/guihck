#include "guihck.h"
#include "guihckElements.h"

#include <stdio.h>
#include <assert.h>

const char TEST_SCM[] =
    "(import (rnrs (6)))"
    "(create-elements!"
    "  (item (list 'id 'scm-1"
    "              'on-key (lambda (k sc a m)"
    "                (display \"on-key 1\\n\"))"
    "              'on-char (lambda (c)"
    "                (display \"on-char 1\\n\")))"
    "    (item (list 'id 'scm-2"
    "                'on-key (lambda (k sc a m)"
    "                  (display \"on-key 2\\n\") #t)"
    "                'on-char (lambda (c)"
    "                  (display \"on-char 2\\n\") #t)))))"

;

typedef struct keyboardProbeData
{
  int* count;
  bool accept;
} keyboardProbeData;

bool keyEvent(guihckContext* ctx, guihckElementId id, guihckKey key, int scancode, guihckKeyAction action, guihckKeyMods mods, void* data)
{
  (void) ctx;
  const char* actionStr =
      action == GUIHCK_KEY_PRESS ? "press" :
      action == GUIHCK_KEY_RELEASE ? "release" :
      action == GUIHCK_KEY_REPEAT ? "repeat" :
      "unknown";

  keyboardProbeData* probeData = data;
  *(probeData->count) += 1;
  printf("keyEvent %d %d %d %s %d, count = %d\n", (int) id, (int) key, scancode, actionStr, (int) mods, *(probeData->count));

  return probeData->accept;
}

bool keyChar(guihckContext* ctx, guihckElementId id, unsigned int codepoint, void* data)
{
  (void) ctx;

  keyboardProbeData* probeData = data;
  *(probeData->count) += 1;
  printf("keyChar %d %u, count = %d\n", (int) id, codepoint, *(probeData->count));

  return probeData->accept;
}

int main(int argc, char** argv)
{
  (void) argc;
  (void) argv;

  guihckInit();
  guihckContext* ctx = guihckContextNew();

  guihckElementTypeFunctionMap keyboardProbeMap = { NULL, NULL, NULL, NULL, keyEvent, keyChar };
  guihckElementTypeAdd(ctx, "keyboard-probe", keyboardProbeMap, sizeof(int*));

  int count = 0;

  guihckStackPushNewElement(ctx, "keyboard-probe");
  guihckElementId id1 = guihckStackGetElement(ctx);
  keyboardProbeData* data1 = guihckElementGetData(ctx, id1);
  data1->count = &count;
  data1->accept = false;

  guihckStackPushNewElement(ctx, "keyboard-probe");
  guihckElementId id2 = guihckStackGetElement(ctx);
  keyboardProbeData* data2 = guihckElementGetData(ctx, id2);
  data2->count = &count;
  data2->accept = false;

  guihckStackPushNewElement(ctx, "keyboard-probe");
  guihckElementId id3 = guihckStackGetElement(ctx);
  keyboardProbeData* data3 = guihckElementGetData(ctx, id3);
  data3->count = &count;
  data3->accept = true;

  guihckStackPushNewElement(ctx, "keyboard-probe");
  guihckElementId id4 = guihckStackGetElement(ctx);
  keyboardProbeData* data4 = guihckElementGetData(ctx, id4);
  data4->count = &count;
  data4->accept = false;

  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 0);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 0);

  guihckContextKeyboardFocus(ctx, id1);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 1);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 1);

  count = 0;
  guihckContextKeyboardFocus(ctx, id2);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 2);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 2);

  count = 0;
  guihckContextKeyboardFocus(ctx, id3);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 1);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 1);

  count = 0;
  guihckContextKeyboardFocus(ctx, id4);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 2);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 2);

  guihckElementsAddItemType(ctx);
  guihckContextExecuteScript(ctx, TEST_SCM);

  count = 0;
  guihckStackPushElementById(ctx, "scm-1");
  guihckElementId scmId1 = guihckStackGetElement(ctx);
  guihckContextKeyboardFocus(ctx, scmId1);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 2);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 2);

  count = 0;
  guihckStackPushElementById(ctx, "scm-2");
  guihckElementId scmId2 = guihckStackGetElement(ctx);
  guihckContextKeyboardFocus(ctx, scmId2);
  guihckContextKeyboardKey(ctx, 0, 0, GUIHCK_KEY_PRESS, 0);
  assert(count == 0);
  count = 0;
  guihckContextKeyboardChar(ctx, 42);
  assert(count == 0);

  guihckContextFree(ctx);

  return EXIT_SUCCESS;
}

