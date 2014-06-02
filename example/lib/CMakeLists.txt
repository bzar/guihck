add_subproject(glhck)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Skip GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Skip GLFW examples")

add_subproject(glfw)
add_subproject(glfwhck)
