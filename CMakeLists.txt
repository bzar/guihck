CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck)

include("cmake/subproject.cmake")

OPTION(GUIHCK_BUILD_TESTS "Build guihck tests/examples" OFF)

set(GLHCK_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLHCK examples")
find_package(PkgConfig REQUIRED)
pkg_check_modules(guile-2.0 REQUIRED guile-2.0)

add_subdirectory(lib)

include_directories(
  include
  ${guile-2.0_INCLUDE_DIRS}
  ${glhck_SOURCE_DIR}/include
  lib/chck/pool
  lib/chck/lut
)

file(GLOB SOURCES src/*.c)

list(APPEND CMAKE_C_FLAGS "-Wall -Wextra -Wno-variadic-macros -Wno-long-long")

add_library(guihck
    ${SOURCES}
)

target_link_libraries(guihck chckPool chckLut ${guile-2.0_LIBRARIES})

if(GUIHCK_BUILD_TESTS)
  add_subdirectory(test)
endif(GUIHCK_BUILD_TESTS)
