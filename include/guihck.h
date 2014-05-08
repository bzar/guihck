#ifndef GUIHCK_H
#define GUIHCK_H

#include <libguile.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
#endif

typedef size_t guihckElementId;
typedef size_t guihckElementTypeId;
typedef size_t guihckMouseAreaId;

#define GUIHCK_NO_PARENT -1

typedef struct _guihckContext guihckContext;
typedef struct _guihckElement guihckElement;

// Element type function map
typedef struct guihckElementTypeFunctionMap {
   void (*init)(guihckContext* ctx, guihckElementId id, void* data);
   void (*destroy)(guihckContext* ctx, guihckElementId id, void* data);
   bool (*update)(guihckContext* ctx, guihckElementId id, void* data);
   void (*render)(guihckContext* ctx, guihckElementId id, void* data);
} guihckElementTypeFunctionMap;

// Mouse area function map
typedef struct guihckMouseAreaFunctionMap {
  bool (*mouseDown)(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
  bool (*mouseUp)(guihckContext* ctx, guihckElementId id, void* data, int button, float x, float y);
  bool (*mouseMove)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
  bool (*mouseEnter)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
  bool (*mouseExit)(guihckContext* ctx, guihckElementId id, void* data, float sx, float sy, float dx, float dy);
} guihckMouseAreaFunctionMap;

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

// Element type

guihckElementTypeId guihckElementTypeAdd(guihckContext* ctx, const char* name, guihckElementTypeFunctionMap functionMap, size_t dataSize);

// Element

guihckElementId guihckElementNew(guihckContext* ctx, guihckElementTypeId type, guihckElementId parentId);
void guihckElementRemove(guihckContext* ctx, guihckElementId id);

SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char *key);
void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value);
guihckElementId guihckElementGetParent(guihckContext* ctx, guihckElementId elementId);

void* guihckElementGetData(guihckContext* ctx, guihckElementId elementId);

// Mouse area

guihckMouseAreaId guihckMouseAreaNew(guihckContext* ctx, guihckElementId elementId, guihckMouseAreaFunctionMap functionMap);
void guihckMouseAreaRemove(guihckContext* ctx, guihckMouseAreaId mouseAreaId);
void guihckMouseAreaRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float x, float y, float width, float height);
void guihckMouseAreaGetRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float* x, float* y, float* width, float* height);


void guihckContextExecuteExpression(guihckContext* ctx, SCM expression);
void guihckContextExecuteScript(guihckContext* ctx, const char* script);
void guihckContextExecuteScriptFile(guihckContext* ctx, const char* path);

// Element stack operations
void guihckContextCreateElement(guihckContext* ctx, const char* typeName);
void guihckContextPushElement(guihckContext* ctx, guihckElementId elementId);
void guihckContextPushElementById(guihckContext* ctx, const char* id);
void guihckContextPopElement(guihckContext* ctx);
SCM guihckContextGetElementProperty(guihckContext* ctx, const char *key);
void guihckContextElementProperty(guihckContext* ctx, const char* key, SCM value);

#ifdef __cplusplus
}
#endif

#endif
