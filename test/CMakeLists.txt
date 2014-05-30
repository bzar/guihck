CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(guihck-tests)

include_directories(
  ../include
  ${guile-2.0_INCLUDE_DIRS}
)

list(APPEND CMAKE_C_FLAGS "-Wall -Wextra -Wno-variadic-macros -Wno-long-long")

add_executable(simple simple.c)
target_link_libraries(simple guihck)
add_test(simple simple)

add_executable(visible visible.c)
target_link_libraries(visible guihck)
add_test(visible visible)

add_executable(listener listener.c)
target_link_libraries(listener guihck)
add_test(listener listener)

add_executable(keyboard keyboard.c)
target_link_libraries(keyboard guihck)
add_test(keyboard keyboard)

add_executable(keybind keybind.c)
target_link_libraries(keybind guihck)
add_test(keybind keybind)

# Pure SCM tests
add_executable(scm-test-runner scm-test-runner.c)
target_link_libraries(scm-test-runner guihck)
add_test(alias scm-test-runner scm/alias.scm)
add_test(bind scm-test-runner scm/bind.scm)
add_test(bound scm-test-runner scm/bound.scm)

FILE(COPY scm DESTINATION .)
