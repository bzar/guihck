CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck)

include(CTest)

include("cmake/subproject.cmake")

OPTION(GUIHCK_BUILD_TESTS "Build guihck tests" ON)
OPTION(GUIHCK_BUILD_EXAMPLES "Build guihck examples" OFF)

set(GLHCK_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLHCK examples")
find_package(PkgConfig REQUIRED)
pkg_check_modules(guile-2.0 REQUIRED guile-2.0)

add_subdirectory(lib)
add_subdirectory(src)

if(GUIHCK_BUILD_TESTS)
  add_subdirectory(test)
endif(GUIHCK_BUILD_TESTS)

if(GUIHCK_BUILD_EXAMPLES)
  add_subdirectory(example)
endif(GUIHCK_BUILD_EXAMPLES)
