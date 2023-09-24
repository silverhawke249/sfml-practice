add_executable(minesweeper WIN32 main.cpp minesweeper.cpp)
target_link_libraries(minesweeper PRIVATE sfml-graphics)

if(WIN32)
    target_link_libraries(minesweeper PRIVATE sfml-main)

    add_custom_command(
        TARGET minesweeper
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<BOOL:${ARCH_64BITS}>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:minesweeper>
        VERBATIM)
endif()

add_custom_command(
    TARGET minesweeper
    COMMENT "Copy resources"
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/tileset.png ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/tileset.png
)
