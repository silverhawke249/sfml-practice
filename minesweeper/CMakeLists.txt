embed_resource(minesweeper-numbers ${CMAKE_CURRENT_SOURCE_DIR}/resources/numbers.png)
embed_resource(minesweeper-tileset ${CMAKE_CURRENT_SOURCE_DIR}/resources/tileset.png)

add_library(minesweeper STATIC Minesweeper.cpp)
target_link_libraries(minesweeper PUBLIC
    minesweeper-numbers
    minesweeper-tileset
    sfml-graphics
)

add_executable(minesweeper-exec WIN32 Main.cpp)
target_link_libraries(minesweeper-exec PRIVATE
    minesweeper
    ImGui-SFML::ImGui-SFML
    sfml-graphics
)
set_target_properties(minesweeper-exec PROPERTIES
    OUTPUT_NAME "minesweeper"
    PREFIX ""
)

if(WIN32)
    target_link_libraries(minesweeper-exec PRIVATE sfml-main)
endif()
