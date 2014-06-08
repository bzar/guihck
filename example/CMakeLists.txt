CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck-examples)

add_subdirectory(lib)

include_directories(
  ../include
  ../lib/chck/pool
  ../lib/chck/lut
  ${guile-2.0_INCLUDE_DIRS}
  ${kazmath_SOURCE_DIR}/src
  ${glhck_SOURCE_DIR}/include
  ${glfwhck_SOURCE_DIR}/include
  ${GLFW_SOURCE_DIR}/include
  ../modules/glhck/include)

list(APPEND CMAKE_C_FLAGS "-Wall -Wextra -Wno-variadic-macros -Wno-long-long")

add_executable(guihck-cli-run cli-run.c)
target_link_libraries(guihck-cli-run guihck)

add_definitions(-DGLHCK_KAZMATH_FLOAT -DUSE_SINGLE_PRECISION)

add_executable(guihck-glhckElements glhckElements.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-glhckElements guihck glhck glfw ${GLFW_LIBRARIES})

add_executable(guihck-script script.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-script guihck glhck glfw ${GLFW_LIBRARIES})

add_executable(guihck-glhck-run guihck-glhck-run.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-glhck-run guihck glhck glfw ${GLFW_LIBRARIES} glfwhck)

add_executable(guihck-functions functions.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-functions guihck glhck glfw ${GLFW_LIBRARIES} glfwhck)

FILE(COPY scm DESTINATION .)
FILE(COPY glhck-scm DESTINATION .)
FILE(COPY img DESTINATION .)
FILE(COPY fonts DESTINATION .)

