embed_resource(Minesweeper-Tileset ${CMAKE_CURRENT_SOURCE_DIR}/resources/tileset.png)

add_library(Minesweeper-Lib STATIC Minesweeper.cpp)
target_link_libraries(Minesweeper-Lib PUBLIC
    Minesweeper-Tileset
    sfml-graphics
)

add_executable(Minesweeper WIN32 Main.cpp)
target_link_libraries(Minesweeper PRIVATE
    Minesweeper-Lib
    ImGui-SFML::ImGui-SFML
    sfml-graphics
)
sfml_action(Minesweeper)
