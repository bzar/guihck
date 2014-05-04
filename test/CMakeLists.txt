CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck-tests)

add_subdirectory(lib)

include_directories(
  ${kazmath_SOURCE_DIR}/src
  ${glhck_SOURCE_DIR}/include
  ${glfwhck_SOURCE_DIR}/include
  ${GLFW_SOURCE_DIR}/include
  ../modules/glhck/include)

add_executable(guihck-simple simple.c)
target_link_libraries(guihck-simple guihck)

add_definitions(-DGLHCK_KAZMATH_FLOAT -DUSE_SINGLE_PRECISION)

add_executable(guihck-glhckElements glhckElements.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-glhckElements guihck glhck glfw ${GLFW_LIBRARIES})

add_executable(guihck-script script.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-script guihck glhck glfw ${GLFW_LIBRARIES})

add_executable(guihck-mouseArea mouseArea.c ../modules/glhck/src/glhckElements.c)
target_link_libraries(guihck-mouseArea guihck glhck glfw ${GLFW_LIBRARIES} glfwhck)

FILE(COPY scm DESTINATION .)

