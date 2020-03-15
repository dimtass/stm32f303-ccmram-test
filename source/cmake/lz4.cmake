set(LZ4_LIB_DIR ${CMAKE_SOURCE_DIR}/libs/lz4)

# Make sure that git submodule is initialized and updated
if (NOT EXISTS "${LZ4_LIB_DIR}")
  message(FATAL_ERROR "minilzo library submodule not found. Initialize with 'git submodule update --init' in the source directory")
endif()

include_directories(
    ${LZ4_LIB_DIR}/inc
)

set(LZ4_LIB_SRC
    ${LZ4_LIB_DIR}/src/lz4.c
)

set_source_files_properties(${LZ4_LIB_SRC}
    PROPERTIES COMPILE_FLAGS ${STM32_DEFINES}
)

add_library(lz4 STATIC ${LZ4_LIB_SRC})

set_target_properties(lz4 PROPERTIES LINKER_LANGUAGE C)

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} lz4)