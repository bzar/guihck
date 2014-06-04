#ifndef GUIHCK_INTERNAL_H
#define GUIHCK_INTERNAL_H

#include "guihck.h"
#include "pool.h"
#include "lut.h"

#define GUIHCK_NO_PARENT SIZE_MAX

#if defined(_MSC_VER)
# define _GUIHCK_TLS __declspec(thread)
# define _GUIHCK_TLS_FOUND
#elif defined(__GNUC__)
# define _GUIHCK_TLS __thread
# define _GUIHCK_TLS_FOUND
#else
# define _GUIHCK_TLS
# warning "No Thread-local storage! Multi-threaded guihck applications may have unexpected behaviour!"
#endif

typedef struct _guihckContext
{
  chckPool* elements;
  chckPool* elementTypes;
  chckHashTable* elementTypesByName;
  chckIterPool* renderOrder;
  bool renderOrderChanged;
  chckPool* mouseAreas;  /* should also have a quadtree for references */
  chckIterPool* stack;
  guihckElementId rootElementId;
  chckPool* propertyListeners;
  guihckElementId focused;
  chckHashTable* keyCodesByName;
  chckHashTable* keyNamesByCode;
  double time;
} _guihckContext;

typedef struct _guihckElementType
{
  char* name;
  guihckElementTypeFunctionMap functionMap;
  size_t dataSize;
} _guihckElementType;

typedef struct _guihckRect
{
    float x;
    float y;
    float w;
    float h;

} _guihckRect;

typedef struct _guihckPropertyListener
{
  guihckElementId listenerId;
  guihckElementId listenedId;
  char* propertyName;
  guihckPropertyListenerCallback callback;
  void* data;
  guihckPropertyListenerFreeCallback freeCallback;
} _guihckPropertyListener;

typedef enum _guihckPropertyType { GUIHCK_PROPERTY_VALUE, GUIHCK_PROPERTY_ALIAS, GUIHCK_PROPERTY_BIND } _guihckPropertyType;
typedef struct _guihckBoundProperty
{
  guihckPropertyListenerId listenerId;
  SCM value;
  size_t index;
} _guihckBoundProperty;

typedef struct _guihckBoundPropertyRef
{
  char* propertyName;
  size_t index;
} _guihckBoundPropertyRef;

typedef struct _guihckProperty
{
  _guihckPropertyType type;
  SCM value;
  chckIterPool* listeners;
  union
  {
    struct
    {
      guihckPropertyListenerId listenerId;
    } alias;
    struct
    {
      SCM function;
      chckIterPool* bound; /* _guihckBoundProperty for each bound property */
    } bind;
  };

} _guihckProperty;

typedef struct _guihckElement
{
  guihckElementTypeId type;
  void* data;
  guihckElementId parent;
  chckIterPool* children;
  chckHashTable* properties;
  chckIterPool* listened;
  bool dirty;
} _guihckElement;

typedef struct _guihckMouseArea
{
  guihckElementId elementId;
  _guihckRect rect;
  guihckMouseAreaFunctionMap functionMap;
} _guihckMouseArea;

#endif
