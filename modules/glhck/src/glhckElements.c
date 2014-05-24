#include "glhckElements.h"
#include "guihckElementUtils.h"
#include "glhck/glhck.h"
#include <stdio.h>

static const char GUIHCK_GLHCK_RECTANGLE_SCM[] =
    "(define (rectangle props . children)"
    "  (create-element 'rectangle (append '(x 0 y 0 width 0 height 0 color (255 255 255)) props) children))";

static const char GUIHCK_GLHCK_TEXT_SCM[] =
    "(define (text props . children)"
    "  (create-element 'text (append '(x 0 y 0 text \"\" size 12 color (255 255 255)) props) children))";

static const char GUIHCK_GLHCK_IMAGE_SCM[] =
    "(define (image props . children)"
    "  (create-element 'image (append '(x 0 y 0 color (255 255 255) source \"\" source-width 0 source-height 0) props) children))";

static void initRectangle(guihckContext* ctx, guihckElementId id, void* data);
static void destroyRectangle(guihckContext* ctx, guihckElementId id, void* data);
static bool updateRectangle(guihckContext* ctx, guihckElementId id, void* data);
static void renderRectangle(guihckContext* ctx, guihckElementId id, void* data);

typedef struct _guihckGlhckText
{
  glhckText* text;
  unsigned int font;
  glhckObject* object;
  char* content;
} _guihckGlhckText;

static void initText(guihckContext* ctx, guihckElementId id, void* data);
static void destroyText(guihckContext* ctx, guihckElementId id, void* data);
static bool updateText(guihckContext* ctx, guihckElementId id, void* data);
static void renderText(guihckContext* ctx, guihckElementId id, void* data);

typedef struct _guihckGlhckImage
{
  glhckObject* object;
  char* source;
} _guihckGlhckImage;

static void initImage(guihckContext* ctx, guihckElementId id, void* data);
static void destroyImage(guihckContext* ctx, guihckElementId id, void* data);
static bool updateImage(guihckContext* ctx, guihckElementId id, void* data);
static void renderImage(guihckContext* ctx, guihckElementId id, void* data);

void guihckGlhckAddAllTypes(guihckContext* ctx)
{
  guihckGlhckAddRectangleType(ctx);
  guihckGlhckAddTextType(ctx);
  guihckGlhckAddImageType(ctx);
}

void guihckGlhckAddRectangleType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initRectangle,
    destroyRectangle,
    updateRectangle,
    renderRectangle,
    NULL,
    NULL
  };
  guihckElementTypeAdd(ctx, "rectangle", functionMap, sizeof(glhckObject*));
  guihckContextExecuteScript(ctx, GUIHCK_GLHCK_RECTANGLE_SCM);
}

void initRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObject* o = glhckPlaneNew(1.0, 1.0);
  glhckMaterial* m = glhckMaterialNew(NULL);
  glhckObjectMaterial(o, m);
  glhckMaterialFree(m);

  *((glhckObject**) data) = o;
  guihckElementAddParentPositionListeners(ctx, id);
  guihckElementAddUpdateProperty(ctx, id, "absolute-x");
  guihckElementAddUpdateProperty(ctx, id, "absolute-y");
  guihckElementAddUpdateProperty(ctx, id, "width");
  guihckElementAddUpdateProperty(ctx, id, "height");
  guihckElementAddUpdateProperty(ctx, id, "color");
}

void destroyRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  glhckObjectFree(*((glhckObject**) data));
}

bool updateRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  glhckObject* o = *((glhckObject**) data);
  SCM x = guihckElementGetProperty(ctx, id, "absolute-x");
  SCM y = guihckElementGetProperty(ctx, id, "absolute-y");
  SCM w = guihckElementGetProperty(ctx, id, "width");
  SCM h = guihckElementGetProperty(ctx, id, "height");
  SCM c = guihckElementGetProperty(ctx, id, "color");

  kmVec3 position = *glhckObjectGetPosition(o);
  kmVec3 scale = *glhckObjectGetScale(o);
  if(scm_is_real(w)) scale.x = scm_to_double(w)/2;
  if(scm_is_real(h)) scale.y = scm_to_double(h)/2;
  if(scm_is_real(x)) position.x = scm_to_double(x) + scale.x;
  if(scm_is_real(y)) position.y = scm_to_double(y) + scale.y;

  glhckObjectPosition(o, &position);
  glhckObjectScale(o, &scale);

  if(scm_to_bool(scm_list_p(c)) && scm_to_int32(scm_length(c)) == 3)
  {
    glhckColorb color = *glhckMaterialGetDiffuse(glhckObjectGetMaterial(o));
    SCM r = scm_list_ref(c, scm_from_int8(0));
    SCM g = scm_list_ref(c, scm_from_int8(1));
    SCM b = scm_list_ref(c, scm_from_int8(2));

    if(scm_to_bool(scm_integer_p(r))) color.r = scm_to_uint8(r);
    if(scm_to_bool(scm_integer_p(g))) color.g = scm_to_uint8(g);
    if(scm_to_bool(scm_integer_p(b))) color.b = scm_to_uint8(b);

    glhckMaterialDiffuse(glhckObjectGetMaterial(o), &color);
  }

  return false;
}

void renderRectangle(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  glhckObjectDraw(*((glhckObject**) data));
}


void guihckGlhckAddTextType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initText,
    destroyText,
    updateText,
    renderText,
    NULL,
    NULL
  };
  guihckElementTypeAdd(ctx, "text", functionMap, sizeof(_guihckGlhckText));
  guihckContextExecuteScript(ctx, GUIHCK_GLHCK_TEXT_SCM);
}

void initText(guihckContext* ctx, guihckElementId id, void* data)
{
  _guihckGlhckText* d = data;
  d->text = glhckTextNew(256, 256);
  d->font = glhckTextFontNewKakwafont(d->text, NULL);
  d->object = glhckPlaneNew(1.0, 1.0);
  glhckMaterial* m = glhckMaterialNew(NULL);
  glhckObjectMaterial(d->object, m);
  glhckMaterialFree(m);
  d->content = NULL;
  guihckElementAddParentPositionListeners(ctx, id);
  guihckElementAddUpdateProperty(ctx, id, "absolute-x");
  guihckElementAddUpdateProperty(ctx, id, "absolute-y");
  guihckElementAddUpdateProperty(ctx, id, "text");
  guihckElementAddUpdateProperty(ctx, id, "size");
  guihckElementAddUpdateProperty(ctx, id, "color");
}

void destroyText(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  _guihckGlhckText* d = data;
  glhckTextFontFree(d->text, d->font);
  glhckTextFree(d->text);
  glhckObjectFree(d->object);
  if(d->content)
    free(d->content);
}

bool updateText(guihckContext* ctx, guihckElementId id, void* data)
{
  _guihckGlhckText* d = data;

  SCM textContent = guihckElementGetProperty(ctx, id, "text");
  SCM textSize = guihckElementGetProperty(ctx, id, "size");
  SCM c = guihckElementGetProperty(ctx, id, "color");

  if(scm_is_string(textContent))
  {
    char* textContentStr = scm_to_utf8_string(textContent);
    if(!d->content || strcmp(textContentStr, d->content))
    {
      float size = scm_is_real(textSize)  ? scm_to_double(textSize) : 12;
      glhckTexture* texture = glhckTextRTT(d->text, d->font, size, textContentStr, glhckTextureDefaultLinearParameters());
      if(texture)
      {
        glhckMaterialTexture(glhckObjectGetMaterial(d->object), texture);
        glhckTextureFree(texture);
      }
      free(d->content);
      d->content = textContentStr;
    }
    else
    {
      free(textContentStr);
    }
  }

  float w = 0;
  float h = 0;
  glhckTexture* texture = glhckMaterialGetTexture(glhckObjectGetMaterial(d->object));
  if(texture)
  {
    int textureWidth, textureHeight;
    glhckTextureGetInformation(texture, NULL, &textureWidth, &textureHeight, NULL, NULL, NULL, NULL);
    w = textureWidth;
    h = textureHeight;
    guihckElementProperty(ctx, id, "width", scm_from_double(w));
    guihckElementProperty(ctx, id, "height", scm_from_double(h));
  }

  SCM x = guihckElementGetProperty(ctx, id, "absolute-x");
  SCM y = guihckElementGetProperty(ctx, id, "absolute-y");
  kmVec3 position = *glhckObjectGetPosition(d->object);
  kmVec3 scale = *glhckObjectGetScale(d->object);

  scale.x = w/2;
  scale.y = h/2;
  if(scm_is_real(x)) position.x = scm_to_double(x) + scale.x;
  if(scm_is_real(y)) position.y = scm_to_double(y) + scale.y;

  glhckObjectPosition(d->object, &position);
  glhckObjectScale(d->object, &scale);

  if(scm_to_bool(scm_list_p(c)) && scm_to_int32(scm_length(c)) == 3)
  {
    glhckColorb color = *glhckMaterialGetDiffuse(glhckObjectGetMaterial(d->object));
    SCM r = scm_list_ref(c, scm_from_int8(0));
    SCM g = scm_list_ref(c, scm_from_int8(1));
    SCM b = scm_list_ref(c, scm_from_int8(2));

    if(scm_is_integer(r)) color.r = scm_to_uint8(r);
    if(scm_is_integer(g)) color.g = scm_to_uint8(g);
    if(scm_is_integer(b)) color.b = scm_to_uint8(b);

    glhckMaterialDiffuse(glhckObjectGetMaterial(d->object), &color);
  }

  return false;
}

void renderText(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  _guihckGlhckText* d = data;
  glhckObjectDraw(d->object);
}

void guihckGlhckAddImageType(guihckContext* ctx)
{
  guihckElementTypeFunctionMap functionMap = {
    initImage,
    destroyImage,
    updateImage,
    renderImage,
    NULL,
    NULL
  };
  guihckElementTypeAdd(ctx, "image", functionMap, sizeof(_guihckGlhckImage));
  guihckContextExecuteScript(ctx, GUIHCK_GLHCK_IMAGE_SCM);
}

void initImage(guihckContext* ctx, guihckElementId id, void* data)
{
  _guihckGlhckImage* d = data;
  d->object = glhckPlaneNew(1.0, 1.0);
  glhckMaterial* m = glhckMaterialNew(NULL);
  glhckObjectMaterial(d->object, m);
  glhckMaterialFree(m);
  d->source = NULL;
  guihckElementAddParentPositionListeners(ctx, id);
  guihckElementAddUpdateProperty(ctx, id, "absolute-x");
  guihckElementAddUpdateProperty(ctx, id, "absolute-y");
  guihckElementAddUpdateProperty(ctx, id, "width");
  guihckElementAddUpdateProperty(ctx, id, "height");
  guihckElementAddUpdateProperty(ctx, id, "color");
  guihckElementAddUpdateProperty(ctx, id, "source");
}

void destroyImage(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  _guihckGlhckImage* d = data;
  glhckObjectFree(d->object);
  if(d->source)
    free(d->source);
}

bool updateImage(guihckContext* ctx, guihckElementId id, void* data)
{
  _guihckGlhckImage* d = data;

  SCM source = guihckElementGetProperty(ctx, id, "source");

  if(scm_is_string(source))
  {
    char* sourceStr = scm_to_utf8_string(source);
    if(!d->source || strcmp(sourceStr, d->source))
    {
      glhckTexture* texture = glhckTextureNewFromFile(sourceStr, glhckImportDefaultImageParameters(), glhckTextureDefaultSpriteParameters());
      glhckMaterialTexture(glhckObjectGetMaterial(d->object), texture);
      glhckTextureFree(texture);
      free(d->source);
      d->source = sourceStr;
      int textureWidth, textureHeight;
      glhckTextureGetInformation(texture, NULL, &textureWidth, &textureHeight, NULL, NULL, NULL, NULL);
      guihckElementProperty(ctx, id, "source-width", scm_from_double(textureWidth));
      guihckElementProperty(ctx, id, "source-height", scm_from_double(textureHeight));

    }
    else
    {
      free(sourceStr);
    }
  }

  SCM width = guihckElementGetProperty(ctx, id, "width");
  SCM height = guihckElementGetProperty(ctx, id, "height");

  if(!scm_is_real(width))
  {
    width = guihckElementGetProperty(ctx, id, "source-width");
    guihckElementProperty(ctx, id, "width", width);
  }

  if(!scm_is_real(height))
  {
    height = guihckElementGetProperty(ctx, id, "source-height");
    guihckElementProperty(ctx, id, "height", height);
  }

  float w = scm_is_real(width) ? scm_to_double(width) : 0;
  float h = scm_is_real(height) ? scm_to_double(height) : 0;

  SCM x = guihckElementGetProperty(ctx, id, "absolute-x");
  SCM y = guihckElementGetProperty(ctx, id, "absolute-y");

  kmVec3 position = *glhckObjectGetPosition(d->object);
  kmVec3 scale = *glhckObjectGetScale(d->object);
  scale.x = w/2;
  scale.y = h/2;
  if(scm_is_real(x)) position.x = scm_to_double(x) + scale.x;
  if(scm_is_real(y)) position.y = scm_to_double(y) + scale.y;

  glhckObjectPosition(d->object, &position);
  glhckObjectScale(d->object, &scale);

  SCM c = guihckElementGetProperty(ctx, id, "color");
  if(scm_to_bool(scm_list_p(c)) && scm_to_int32(scm_length(c)) == 3)
  {
    glhckColorb color = *glhckMaterialGetDiffuse(glhckObjectGetMaterial(d->object));
    SCM r = scm_list_ref(c, scm_from_int8(0));
    SCM g = scm_list_ref(c, scm_from_int8(1));
    SCM b = scm_list_ref(c, scm_from_int8(2));

    if(scm_to_bool(scm_integer_p(r))) color.r = scm_to_uint8(r);
    if(scm_to_bool(scm_integer_p(g))) color.g = scm_to_uint8(g);
    if(scm_to_bool(scm_integer_p(b))) color.b = scm_to_uint8(b);

    glhckMaterialDiffuse(glhckObjectGetMaterial(d->object), &color);
  }

  return false;
}

void renderImage(guihckContext* ctx, guihckElementId id, void* data)
{
  (void) ctx;
  (void) id;

  _guihckGlhckImage* d = data;
  glhckObjectDraw(d->object);
}
