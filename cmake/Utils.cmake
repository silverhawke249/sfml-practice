enable_language(ASM)

function(embed_resource TARGET INPUT_FILE)
    set(SYMBOL_NAME ${TARGET})
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SYMBOL_NAME ${SYMBOL_NAME})

    file(GENERATE
        OUTPUT gen/${TARGET}-embedded.S
        CONTENT "    .global ${SYMBOL_NAME}_Begin
    .global ${SYMBOL_NAME}_End
    .section        .rodata
${SYMBOL_NAME}_Begin:
    .align 4
    .incbin \"${INPUT_FILE}\"
${SYMBOL_NAME}_End:")

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

function(sfml_action TARGET)
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE sfml-main)

        add_custom_command(
            TARGET ${TARGET}
            COMMENT "Copy OpenAL DLL for ${TARGET}"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<BOOL:${ARCH_64BITS}>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:${TARGET}>
            VERBATIM)
    endif()
endfunction()
