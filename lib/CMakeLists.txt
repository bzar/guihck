file(GLOB CHCK_POOL_SOURCES chck/pool/pool.c)
add_library(chckPool ${CHCK_POOL_SOURCES})

file(GLOB CHCK_LUT_SOURCES chck/lut/lut.c)
add_library(chckLut ${CHCK_LUT_SOURCES})
