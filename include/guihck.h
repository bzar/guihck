#ifndef GUIHCK_H
#define GUIHCK_H

#include <libguile.h>
#include <stdbool.h>
#include <guihckKeys.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t guihckElementId;
typedef size_t guihckElementTypeId;
typedef size_t guihckMouseAreaId;
typedef size_t guihckPropertyListenerId;

typedef enum guihckKeyAction {
  GUIHCK_KEY_PRESS = GUIHCK_PRESS,
  GUIHCK_KEY_RELEASE = GUIHCK_RELEASE,
  GUIHCK_KEY_REPEAT = GUIHCK_REPEAT
} guihckKeyAction;

typedef int guihckKey;
typedef int guihckKeyMods;

typedef struct _guihckContext guihckContext;
typedef struct _guihckElement guihckElement;

// Element type function map
typedef struct guihckElementTypeFunctionMap {
   void (*init)(guihckContext* ctx, guihckElementId id, void* data);
   void (*destroy)(guihckContext* ctx, guihckElementId id, void* data);
   bool (*update)(guihckContext* ctx, guihckElementId id, void* data);
   void (*render)(guihckContext* ctx, guihckElementId id, void* data);
   bool (*keyEvent)(guihckContext* ctx, guihckElementId id, guihckKey key, int scancode, guihckKeyAction action, guihckKeyMods mods, void* data);
   bool (*keyChar)(guihckContext* ctx, guihckElementId id, unsigned int codepoint, void* data);
} guihckElementTypeFunctionMap;

// Mouse area function map
typedef struct guihckMouseAreaFunctionMap {
  bool (*mouseDown)(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
  bool (*mouseUp)(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
  bool (*mouseMove)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
  bool (*mouseEnter)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
  bool (*mouseExit)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
} guihckMouseAreaFunctionMap;

typedef void (*guihckPropertyListenerCallback)(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);
typedef void (*guihckPropertyListenerFreeCallback)(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId, const char* property, SCM value, void* data);

// Init
void guihckInit();
void guihckRegisterFunction(const char* name, int req, int opt, int rst, scm_t_subr func);

// Context

guihckContext* guihckContextNew();
void guihckContextFree(guihckContext* ctx);
void guihckContextUpdate(guihckContext* ctx);
void guihckContextRender(guihckContext* ctx);

void guihckContextMouseDown(guihckContext* ctx, float x, float y, int button);
void guihckContextMouseUp(guihckContext* ctx, float x, float y, int button);
void guihckContextMouseMove(guihckContext* ctx, float sx, float sy, float dx, float dy);

void guihckContextKeyboardFocus(guihckContext* ctx, guihckElementId elementId);
guihckElementId guihckContextGetKeyboardFocus(guihckContext* ctx);

void guihckContextAddKeyBinding(guihckContext* ctx, guihckKey keyCode, const char* keyName);
const char* guihckContextGetKeyName(guihckContext* ctx, guihckKey keyCode);
guihckKey guihckContextGetKeyCode(guihckContext* ctx, const char* keyName);

void guihckContextKeyboardKey(guihckContext* ctx, guihckKey key, int scancode, guihckKeyAction action, guihckKeyMods mods);
void guihckContextKeyboardChar(guihckContext* ctx, unsigned int codepoint);

void guihckContextTime(guihckContext* ctx, double time);
double guihckContextGetTime(guihckContext* ctx);

guihckElementId guihckContextGetRootElement(guihckContext* ctx);

// Element type

guihckElementTypeId guihckElementTypeAdd(guihckContext* ctx, const char* name, guihckElementTypeFunctionMap functionMap, size_t dataSize);

// Element

guihckElementId guihckElementNew(guihckContext* ctx, guihckElementTypeId type, guihckElementId parentId);
void guihckElementRemove(guihckContext* ctx, guihckElementId id);

SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char *key);
void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value);

guihckElementId guihckElementGetParent(guihckContext* ctx, guihckElementId elementId);
size_t guihckElementGetChildCount(guihckContext* ctx, guihckElementId elementId);
guihckElementId guihckElementGetChild(guihckContext* ctx, guihckElementId elementId, int childIndex);
void guihckElementGetChildren(guihckContext* ctx, guihckElementId elementId, guihckElementId* children);
void guihckElementDirty(guihckContext* ctx, guihckElementId elementId);

void* guihckElementGetData(guihckContext* ctx, guihckElementId elementId);

guihckPropertyListenerId guihckElementAddListener(guihckContext* ctx, guihckElementId listenerId, guihckElementId listenedId,
                                                  const char* propertyName, guihckPropertyListenerCallback callback, void* data,
                                                  guihckPropertyListenerFreeCallback freeCallback);
void guihckElementRemoveListener(guihckContext* ctx, guihckPropertyListenerId propertyListenerId);
bool guihckElementGetVisible(guihckContext* ctx, guihckElementId elementId);
void guihckElementVisible(guihckContext* ctx, guihckElementId elementId, bool value);
// Mouse area

guihckMouseAreaId guihckMouseAreaNew(guihckContext* ctx, guihckElementId elementId, guihckMouseAreaFunctionMap functionMap);
void guihckMouseAreaRemove(guihckContext* ctx, guihckMouseAreaId mouseAreaId);
void guihckMouseAreaRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float x, float y, float width, float height);
void guihckMouseAreaGetRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float* x, float* y, float* width, float* height);


SCM guihckContextExecuteExpression(guihckContext* ctx, SCM expression);
SCM guihckContextExecuteScript(guihckContext* ctx, const char* script);
SCM guihckContextExecuteScriptFile(guihckContext* ctx, const char* path);

// Element stack operations
void guihckStackPushNewElement(guihckContext* ctx, const char* typeName);
void guihckStackPushElement(guihckContext* ctx, guihckElementId elementId);
void guihckStackPushElementById(guihckContext* ctx, const char* id);
void guihckStackPushParentElement(guihckContext* ctx);
void guihckStackPushChildElement(guihckContext* ctx, int childIndex);
void guihckStackPopElement(guihckContext* ctx);
guihckElementId guihckStackGetElement(guihckContext* ctx);
SCM guihckStackGetElementProperty(guihckContext* ctx, const char *key);
void guihckStackElementProperty(guihckContext* ctx, const char* key, SCM value);
int guihckStackGetElementChildCount(guihckContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
