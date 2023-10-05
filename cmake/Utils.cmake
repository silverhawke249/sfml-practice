enable_language(ASM)

function(embed_resource TARGET INPUT_FILE)
    set(SYMBOL_NAME ${TARGET})
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SYMBOL_NAME ${SYMBOL_NAME})

    if(UNIX)
        file(GENERATE
            OUTPUT gen/${TARGET}-embedded.S
            CONTENT "    .global ${SYMBOL_NAME}_Begin
    .global ${SYMBOL_NAME}_End
    .section .note.GNU-stack,\"\",@progbits
    .rodata
${SYMBOL_NAME}_Begin:
    .align 4
    .incbin \"${INPUT_FILE}\"
${SYMBOL_NAME}_End:")
    else()
        file(GENERATE
            OUTPUT gen/${TARGET}-embedded.S
            CONTENT "    .global ${SYMBOL_NAME}_Begin
    .global ${SYMBOL_NAME}_End
    .section .rodata
${SYMBOL_NAME}_Begin:
    .align 4
    .incbin \"${INPUT_FILE}\"
${SYMBOL_NAME}_End:")
    endif()

    file(GENERATE
        OUTPUT gen/${TARGET}-embedded.h
        CONTENT "#pragma once

#include <cstdint>
#include <span>

extern \"C\" uint8_t const ${SYMBOL_NAME}_Begin;
extern \"C\" uint8_t const ${SYMBOL_NAME}_End;

static std::span<uint8_t const> ${SYMBOL_NAME} {&${SYMBOL_NAME}_Begin, &${SYMBOL_NAME}_End};
")

    add_library(${TARGET} STATIC ${CMAKE_CURRENT_BINARY_DIR}/gen/${TARGET}-embedded.S)
    target_compile_options(${TARGET} INTERFACE -include${CMAKE_CURRENT_BINARY_DIR}/gen/${TARGET}-embedded.h)
endfunction()
