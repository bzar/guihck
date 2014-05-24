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

add_executable(listener listener.c)
target_link_libraries(listener guihck)
add_test(listener listener)

add_executable(alias alias.c)
target_link_libraries(alias guihck)
add_test(alias alias)

add_executable(bind bind.c)
target_link_libraries(bind guihck)
add_test(bind bind)

add_executable(keyboard keyboard.c)
target_link_libraries(keyboard guihck)
add_test(keyboard keyboard)
