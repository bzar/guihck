include_directories(
  ../include
  ${guile-2.0_INCLUDE_DIRS}
  ../lib/chck/pool
  ../lib/chck/lut
)

file(GLOB SOURCES *.c)

list(APPEND CMAKE_C_FLAGS "-Wall -Wextra -Wno-variadic-macros -Wno-long-long")

add_library(guihck
    ${SOURCES}
)

target_link_libraries(guihck chckPool chckLut ${guile-2.0_LIBRARIES})
