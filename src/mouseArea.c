#include "internal.h"

static bool pointInRect(float x, float y, const _guihckRect* r);
static chckIterPool* queryMouseAreasContainingPoint(guihckContext* ctx, float x, float y);
static chckIterPool* queryMouseAreasIntersectingLine(guihckContext* ctx, float sx, float sy, float dx, float dy);

void guihckContextMouseDown(guihckContext* ctx, float x, float y, int button)
{
  chckPoolIndex iter = 0;
  guihckMouseAreaId* mouseAreaId = NULL;
  chckIterPool* mouseAreas = queryMouseAreasContainingPoint(ctx, x, y);
  bool handled = false;
  while(!handled && (mouseAreaId = chckIterPoolIter(mouseAreas, &iter)))
  {
    _guihckMouseArea* mouseArea = chckPoolGet(ctx->mouseAreas, *mouseAreaId);
    if(mouseArea->functionMap.mouseDown)
    {
      handled = mouseArea->functionMap.mouseDown(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
  free(mouseAreas);
}


void guihckContextMouseUp(guihckContext* ctx, float x, float y, int button)
{
  chckPoolIndex iter = 0;
  guihckMouseAreaId* mouseAreaId = NULL;
  chckIterPool* mouseAreas = queryMouseAreasContainingPoint(ctx, x, y);
  bool handled = false;
  while(!handled && (mouseAreaId = chckIterPoolIter(mouseAreas, &iter)))
  {
    _guihckMouseArea* mouseArea = chckPoolGet(ctx->mouseAreas, *mouseAreaId);
    if(mouseArea->functionMap.mouseUp)
    {
      handled = mouseArea->functionMap.mouseUp(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), button, x, y);
    }
  }
  free(mouseAreas);
}

void guihckContextMouseMove(guihckContext* ctx, float sx, float sy, float dx, float dy)
{
  chckPoolIndex iter = 0;
  guihckMouseAreaId* mouseAreaId = NULL;
  chckIterPool* mouseAreas = queryMouseAreasIntersectingLine(ctx, sx, sy, dx, dy);
  bool handled = false;
  while(!handled && (mouseAreaId = chckIterPoolIter(mouseAreas, &iter)))
  {
    _guihckMouseArea* mouseArea = chckPoolGet(ctx->mouseAreas, *mouseAreaId);
    if(!mouseArea->functionMap.mouseMove && !mouseArea->functionMap.mouseEnter && !mouseArea->functionMap.mouseExit)
      continue;

    bool s = pointInRect(sx, sy, &mouseArea->rect);
    bool d = pointInRect(dx, dy, &mouseArea->rect);

    if(s && d)
    {
      if(mouseArea->functionMap.mouseMove)
        handled = mouseArea->functionMap.mouseMove(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
    else if(s)
    {
      if(mouseArea->functionMap.mouseExit)
        handled = mouseArea->functionMap.mouseExit(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
    else if(d)
    {
      if(mouseArea->functionMap.mouseEnter)
        handled = mouseArea->functionMap.mouseEnter(ctx, mouseArea->elementId, guihckElementGetData(ctx, mouseArea->elementId), sx, sy, dx, dy);
    }
  }
  free(mouseAreas);
}


guihckMouseAreaId guihckMouseAreaNew(guihckContext* ctx, guihckElementId elementId, guihckMouseAreaFunctionMap functionMap)
{
  _guihckMouseArea mouseArea;
  mouseArea.elementId = elementId;
  mouseArea.rect.x = 0;
  mouseArea.rect.y = 0;
  mouseArea.rect.w = 0;
  mouseArea.rect.h = 0;
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
    mouseArea->rect.x = x;
    mouseArea->rect.y = y;
    mouseArea->rect.w = width;
    mouseArea->rect.h = height;
  }
}


void guihckMouseAreaGetRect(guihckContext* ctx, guihckMouseAreaId mouseAreaId, float* x, float* y, float* width, float* height)
{
  _guihckMouseArea* mouseArea = (_guihckMouseArea*) chckPoolGet(ctx->mouseAreas, mouseAreaId);
  if(mouseArea)
  {
    if(x) *x = mouseArea->rect.x;
    if(y) *y = mouseArea->rect.y;
    if(width) *width = mouseArea->rect.w;
    if(height) *height = mouseArea->rect.h;
  }
}

bool pointInRect(float x, float y, const _guihckRect* r)
{
  return x >= r->x && x <= r->x + r->w && y >= r->y && y <= r->y + r->h;
}

chckIterPool* queryMouseAreasContainingPoint(guihckContext* ctx, float x, float y)
{
  chckIterPool* result = chckIterPoolNew(4, 4, sizeof(guihckMouseAreaId));
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while((mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter)))
  {
    /* Should be replaced by querying a quad tree*/
    if(pointInRect(x, y, &mouseArea->rect))
    {
      guihckMouseAreaId id = mouseAreaIter - 1;
      chckIterPoolAdd(result, &id, NULL);
    }
  }

  return result;
}

chckIterPool* queryMouseAreasIntersectingLine(guihckContext* ctx, float sx, float sy, float dx, float dy)
{
  chckIterPool* result = chckIterPoolNew(4, 4, sizeof(guihckMouseAreaId));
  guihckMouseAreaId mouseAreaIter = 0;
  _guihckMouseArea* mouseArea  = NULL;
  while((mouseArea = chckPoolIter(ctx->mouseAreas, &mouseAreaIter)))
  {
    /* Should be replaced by querying a quad tree*/
    if(pointInRect(sx, sy, &mouseArea->rect)
       || pointInRect(dx, dy, &mouseArea->rect))
    {
      guihckMouseAreaId id = mouseAreaIter - 1;
      chckIterPoolAdd(result, &id, NULL);
    }
  }

  return result;
}
