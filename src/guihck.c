#include "guihck.h"
#include "guihckGuile.h"
#include "pool.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// temporary
typedef struct _guihckKeyValueMapping
{
  char* key;
  SCM value;
} _guihckKeyValueMapping;

typedef struct _guihckContext
{
  chckPool* elements; /* should have trie by name */
  chckPool* elementTypes;
  chckPool* mouseAreas;  /* should also have a quadtree for references */
} _guihckContext;

typedef struct _guihckElementType
{
  char* name;
  guihckElementTypeFunctionMap functionMap;
  size_t dataSize;
} _guihckElementType;

typedef struct _guihckElement
{
  guihckContext* ctx;
  guihckElementTypeId type;
  void* data;
  guihckElementId parent;
  chckIterPool* children;
  chckIterPool* properties; /* Should change to trie */
  char dirty;
} _guihckElement;

typedef struct _guihckMouseArea
{
  guihckElementId elementId;
  float x;
  float y;
  float w;
  float h;
  guihckMouseAreaFunctionMap functionMap;
} _guihckMouseArea;


typedef struct _guihckGui
{
  guihckContext* ctx;
  chckIterPool* stack;
} _guihckGui;

guihckContext* guihckContextNew()
{
  guihckContext* ctx = calloc(1, sizeof(guihckContext));
  ctx->elements = chckPoolNew(64, 64, sizeof(guihckElement));
  ctx->elementTypes = chckPoolNew(16, 16, sizeof(_guihckElementType));
  ctx->mouseAreas= chckPoolNew(16, 16, sizeof(_guihckMouseArea));
  return ctx;
}


void guihckContextFree(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
    if(type->functionMap.destroy)
      type->functionMap.destroy(ctx, iter - 1, current->data); /* id = iter - 1 */
    chckIterPoolFree(current->children);
    chckIterPoolFree(current->properties);
    free(current->data);
  }

  chckPoolFree(ctx->elements);
  chckPoolFree(ctx->mouseAreas);
  free(ctx);
}

void guihckContextUpdate(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    if(current->dirty)
    {
      _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
      assert(type && "Invalid element type");
      if(type->functionMap.update)
        current->dirty = type->functionMap.update(ctx, iter - 1, current->data); /* id = iterator - 1 */
    }
  }
}


void guihckContextRender(guihckContext* ctx)
{
  chckPoolIndex iter = 0;
  guihckElement* current;
  while ((current = chckPoolIter(ctx->elements, &iter)))
  {
    _guihckElementType* type = chckPoolGet(ctx->elementTypes, current->type);
    if(type->functionMap.render)
      type->functionMap.render(ctx, iter - 1, current->data); /* id = iterator - 1 */
  }
}

guihckElementTypeId guihckElementTypeAdd(guihckContext* ctx, const char* name, guihckElementTypeFunctionMap functionMap, size_t dataSize)
{
  chckPoolIndex iter = 0;
  _guihckElementType* current;
  while ((current = chckPoolIter(ctx->elementTypes, &iter)))
  {
    assert(strcmp(current->name, name) != 0 && "Element type with this name already exists");
  }

  _guihckElementType type;
  type.name = strdup(name);
  type.functionMap = functionMap;
  type.dataSize = dataSize;

  guihckElementTypeId id = -1;
  chckPoolAdd(ctx->elementTypes, &type, &id);

  return id;
}

guihckElementId guihckElementNew(guihckContext* ctx, guihckElementTypeId typeId, guihckElementId parentId)
{
  _guihckElementType* type = chckPoolGet(ctx->elementTypes, typeId);
  guihckElement element;
  element.type = typeId;
  element.ctx = ctx;
  element.data = type->dataSize > 0 ? calloc(1, type->dataSize) : NULL;
  element.parent = parentId;
  element.children = chckIterPoolNew(8, 8, sizeof(guihckElementId));
  element.properties = chckIterPoolNew(8, 8, sizeof(_guihckKeyValueMapping));
  element.dirty = 1;

  guihckElementId id = -1;
  chckPoolAdd(ctx->elements, &element, &id);


  guihckElement* parent = chckPoolGet(ctx->elements, parentId);
  if(parent)
    chckIterPoolAdd(parent->children, &id, NULL);

  if(type->functionMap.init)
    type->functionMap.init(ctx, id, element.data);

  return id;
}


void guihckElementRemove(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  _guihckElementType* type = chckPoolGet(ctx->elementTypes, element->type);
  if(type->functionMap.destroy)
    type->functionMap.destroy(ctx, elementId, element->data);
  chckIterPoolFree(element->properties);
  if(element->data)
    free(element->data);

  chckIterPool* children = element->children; /* element may be invalidated while removing children  */
  chckPoolIndex iter = 0;
  guihckElementId* current;
  while ((current = chckIterPoolIter(children, &iter)))
    guihckElementRemove(ctx, *current);
  chckIterPoolFree(children);

  chckPoolRemove(ctx->elements, elementId);
}


SCM guihckElementGetProperty(guihckContext* ctx, guihckElementId elementId, const char* key)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  chckPoolIndex iter = 0;
  _guihckKeyValueMapping* current;
  while ((current = chckIterPoolIter(element->properties, &iter)))
  {
    if(strcmp(current->key, key) == 0)
    {
      return current->value;
    }
  }

  return SCM_UNDEFINED;
}


void guihckElementProperty(guihckContext* ctx, guihckElementId elementId, const char* key, SCM value)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  chckPoolIndex iter = 0;
  _guihckKeyValueMapping* current;
  while ((current = chckIterPoolIter(element->properties, &iter)))
  {
    if(strcmp(current->key, key) == 0)
    {
      if(!scm_equal_p(current->value, value))
      {
        current->value = value;
        element->dirty = 1;
      }
      return;
    }
  }

  _guihckKeyValueMapping mapping;
  mapping.key = strdup(key);
  mapping.value = value;

  chckIterPoolAdd(element->properties, &mapping, NULL);
}


guihckElementId guihckElementGetParent(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  return element->parent;
}

void* guihckElementGetData(guihckContext* ctx, guihckElementId elementId)
{
  guihckElement* element = chckPoolGet(ctx->elements, elementId);
  return element->data;
}

guihckGui* guihckGuiNew()
{
  guihckGui* gui = calloc(1, sizeof(guihckGui));
  gui->ctx = guihckContextNew();
  gui->stack = chckIterPoolNew(8, 8, sizeof(guihckElementId));
  return gui;
}


void guihckGuiFree(guihckGui* gui)
{
  guihckContextFree(gui->ctx);
  chckIterPoolFree(gui->stack);
  free(gui);
}

guihckContext*guihckGuiGetContext(guihckGui* gui)
{
  return gui->ctx;
}

void guihckGuiUpdate(guihckGui* gui)
{
  guihckContextUpdate(gui->ctx);
}

void guihckGuiRender(guihckGui* gui)
{
  guihckContextRender(gui->ctx);
}

void guihckGuiExecuteScript(guihckGui* gui, const char* script)
{
  guihckGuiRunGuile(gui, script);
}

void guihckGuiExecuteScriptFile(guihckGui* gui, const char* path)
{
  FILE* file;

  if(file = fopen(path, "rb"))
  {
    /* Read file into memory */
    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* contents = calloc(pos, 1);
    fread(contents, pos, 1, file);
    fclose(file);

    guihckGuiExecuteScript(gui, contents);
    free(contents);
  }
}

void guihckGuiCreateElement(guihckGui* gui, const char* typeName)
{
  guihckElementId parentId = GUIHCK_NO_PARENT;
  if(chckIterPoolCount(gui->stack) > 0)
  {
    parentId = *((guihckElementId*) chckIterPoolGetLast(gui->stack));
  }

  guihckElementTypeId typeId = 0;
  _guihckElementType* type  = NULL;
  while (type = chckPoolIter(gui->ctx->elementTypes, &typeId))
  {
    if(strcmp(type->name, typeName) == 0)
    {
      typeId -= 1; /* id = iterator value - 1 */
      break;
    }
  }

  assert(type && "Element type not found");

  guihckElementId id = guihckElementNew(gui->ctx, typeId, parentId);
  chckIterPoolAdd(gui->stack, &id, NULL);
}


void guihckGuiPopElement(guihckGui* gui)
{
  size_t size = chckIterPoolCount(gui->stack);
  assert(size > 0 && "Element stack is empty");
  chckIterPoolRemove(gui->stack, size - 1);
}


SCM guihckGuiGetElementProperty(guihckGui* gui, const char* key)
{
  guihckElementId* id = chckIterPoolGetLast(gui->stack);
  assert(id && "Element stack is empty");

  return guihckElementGetProperty(gui->ctx, *id,  key);
}


void guihckGuiElementProperty(guihckGui* gui, const char* key, SCM value)
{
  guihckElementId* id = chckIterPoolGetLast(gui->stack);
  assert(id && "Element stack is empty");

  return guihckElementProperty(gui->ctx, *id,  key, value);
}







static char pointInRect(float x, float y, float rx, float ry, float rw, float rh)
{
  return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void guihckContextMouseDown(guihckContext* ctx, float x, float y, int button)
{
  /* Should be replaced by querying a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(mouseArea->functionMap.mouseDown
       && pointInRect(x, y, mouseArea->x, mouseArea->y, mouseArea->w, mouseArea->h))
    {
      mouseArea->functionMap.mouseDown(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
}


void guihckContextMouseUp(guihckContext* ctx, float x, float y, int button)
{
  /* Should be replaced by querying a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(mouseArea->functionMap.mouseUp
       && pointInRect(x, y, mouseArea->x, mouseArea->y, mouseArea->w, mouseArea->h))
    {
      mouseArea->functionMap.mouseUp(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
}


void guihckContextMouseMove(guihckContext* ctx, float sx, float sy, float dx, float dy)
{
  /* Should be replaced by querying the segment from a quad tree*/
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while (mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter))
  {
    if(mouseArea->functionMap.mouseMove
       && pointInRect(dx, dy, mouseArea->x, mouseArea->y, mouseArea->w, mouseArea->h))
    {
      mouseArea->functionMap.mouseMove(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
  }
}


guihckMouseAreaId guihckMouseAreaNew(guihckContext* ctx, guihckElementId elementId, guihckMouseAreaFunctionMap functionMap)
{
  _guihckMouseArea mouseArea;
  mouseArea.elementId = elementId;
  mouseArea.x = 0;
  mouseArea.y = 0;
  mouseArea.w = 0;
  mouseArea.x = 0;
  mouseArea.functionMap = functionMap;
  guihckMouseAreaId id;
  chckPoolAdd(ctx->mouseAreas, &mouseArea, &id);
  return id;
}


void guihckMouseAreaRemove(guihckContext* ctx, guihckMouseAreaId mouseAreaId)
{
  chckPoolRemove(ctx->mouseAreas, mouseAreaId);
}


void guihckMouseAreaRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float x, float y, float width, float height)
{
  _guihckMouseArea* mouseArea = (_guihckMouseArea*) chckPoolGet(ctx->mouseAreas, mouseAreaId);
  if(mouseArea)
  {
    mouseArea->x = x;
    mouseArea->y = y;
    mouseArea->w = width;
    mouseArea->h = height;
  }
}


void guihckMouseAreaGetRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float* x, float* y, float* width, float* height)
{
  _guihckMouseArea* mouseArea = (_guihckMouseArea*) chckPoolGet(ctx->mouseAreas, mouseAreaId);
  if(mouseArea)
  {
    if(x) *x = mouseArea->x;
    if(y) *y = mouseArea->y;
    if(width) *width = mouseArea->w;
    if(height) *height = mouseArea->h;
  }
}
