CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck-tests)

add_subdirectory(lib)

include_directories(
  ${kazmath_SOURCE_DIR}/src
  ${glhck_SOURCE_DIR}/include
  ${GLFW_SOURCE_DIR}/include
)

add_executable(guihck-simple simple.c)
target_link_libraries(guihck-simple guihck)

add_executable(guihck-glhckElements glhckElements.c)
target_link_libraries(guihck-glhckElements guihck glhck glfw ${GLFW_LIBRARIES})

add_executable(guihck-script script.c)
target_link_libraries(guihck-script guihck glhck glfw ${GLFW_LIBRARIES})

FILE(COPY scm DESTINATION .)

