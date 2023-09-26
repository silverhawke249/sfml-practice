#include "Minesweeper.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

// TODO: Add timer and counter
// TODO: Store high scores

constexpr uint32_t MAX_BOARD_SIZE {600};
constexpr uint32_t MARGIN {25};
constexpr uint32_t PANE_HEIGHT {100};
constexpr uint32_t WINDOW_WIDTH {MAX_BOARD_SIZE + 2 * MARGIN};
constexpr uint32_t WINDOW_HEIGHT {MAX_BOARD_SIZE + 2 * MARGIN + PANE_HEIGHT};
constexpr char const* WINDOW_TITLE {"Minesweeper!"};
constexpr uint32_t BACKGROUND_COLOR {0x1B0345FF};

class MainApp
{
private:
    sf::RenderWindow window;

    GameBoard gameBoard;
    bool lmbHeld {false};

public:
    MainApp(uint32_t boardWidth = 16, uint32_t boardHeight = 16, uint32_t mineCount = 40):
        window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE, sf::Style::Default ^ sf::Style::Resize),
        gameBoard(boardWidth, boardHeight, mineCount)
    {
        consoleLog("Initializing app...");
        this->window.setFramerateLimit(60);
        std::ignore = ImGui::SFML::Init(this->window);
    }

    ~MainApp()
    {
        ImGui::SFML::Shutdown();
    }

    void operator()()
    {
        auto [boardWidth, boardHeight] = this->gameBoard.getBoardDimensions();
        float scalingFactor {MAX_BOARD_SIZE / static_cast<float>(boardWidth)};
        if (boardHeight * scalingFactor > MAX_BOARD_SIZE)
            scalingFactor = MAX_BOARD_SIZE / static_cast<float>(boardHeight);

        auto scaledWidth  = boardWidth * scalingFactor;
        auto scaledHeight = boardHeight * scalingFactor;

        auto offsetX      = static_cast<float>(MAX_BOARD_SIZE - scaledWidth) / 2 + MARGIN;
        auto offsetY      = static_cast<float>(MAX_BOARD_SIZE - scaledHeight) / 2 + MARGIN + PANE_HEIGHT;

        consoleLog("Board size: " + std::to_string(scaledWidth) + " x " + std::to_string(scaledHeight));
        consoleLog("Board offset: " + std::to_string(offsetX) + ", " + std::to_string(offsetY));
        consoleLog("Scaling factor: " + std::to_string(scalingFactor));

        sf::Transform transform;
        transform.translate(offsetX, offsetY).scale(scalingFactor, scalingFactor, 0, 0);

        consoleLog("Starting event loop...");
        sf::Clock deltaClock;
        sf::Event event;
        float boardX;
        float boardY;
        while (this->window.isOpen())
        {
            while (this->window.pollEvent(event))
            {
                ImGui::SFML::ProcessEvent(this->window, event);
                auto imguiMouseCap = ImGui::GetIO().WantCaptureMouse;

                switch (event.type)
                {
                case sf::Event::Closed:
                    this->window.close();
                    break;
                case sf::Event::MouseButtonPressed:
                    if (imguiMouseCap)
                        continue;
                    if (event.mouseButton.button == sf::Mouse::Button::Left)
                    {
                        this->lmbHeld = true;
                        boardX        = (event.mouseButton.x - offsetX) / scalingFactor / TILE_SIZE;
                        boardY        = (event.mouseButton.y - offsetY) / scalingFactor / TILE_SIZE;
                        this->gameBoard.telegraph(boardX, boardY);
                    }
                    break;
                case sf::Event::MouseMoved:
                    if (imguiMouseCap)
                        continue;
                    if (this->lmbHeld)
                    {
                        boardX = (event.mouseMove.x - offsetX) / scalingFactor / TILE_SIZE;
                        boardY = (event.mouseMove.y - offsetY) / scalingFactor / TILE_SIZE;
                        this->gameBoard.telegraph(boardX, boardY);
                    }
                    break;
                case sf::Event::MouseButtonReleased:
                    if (imguiMouseCap)
                        continue;
                    this->lmbHeld = false;
                    this->gameBoard.clearTelegraph();
                    switch (this->gameBoard.getGameState())
                    {
                    case GameState::GAME_NOT_STARTED:
                    case GameState::GAME_ONGOING:
                        boardX = (event.mouseButton.x - offsetX) / scalingFactor / TILE_SIZE;
                        boardY = (event.mouseButton.y - offsetY) / scalingFactor / TILE_SIZE;
                        this->gameBoard.interact(boardX, boardY, event.mouseButton.button);
                        break;
                    case GameState::GAME_WON:
                    case GameState::GAME_LOST:
                        this->gameBoard.initialize();
                        break;
                    }
                    break;
                default:
                    break;
                }
            }

            ImGui::SFML::Update(this->window, deltaClock.restart());

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("New Game"))
                {
                    if (ImGui::MenuItem("Beginner (9x9)"))
                    {
                    }
                    if (ImGui::MenuItem("Intermediate (16x16)"))
                    {
                    }
                    if (ImGui::MenuItem("Expert (30x16)"))
                    {
                    }

                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Custom..."))
                    {
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            this->window.clear(sf::Color(BACKGROUND_COLOR));

            this->window.draw(gameBoard, transform);
            ImGui::SFML::Render(this->window);

            this->window.display();
        }
    }
};

int main(int argc, char* argv[])
{
    consoleLog("SFML-Minesweeper by silverhawke");
    consoleLog("Working directory: " + std::filesystem::current_path().string());
    consoleLog("-------------------------------");

    if (argc == 4)
    {
        auto width     = std::stoi(argv[1]);
        auto height    = std::stoi(argv[2]);
        auto mineCount = std::stoi(argv[3]);
        if (width > 0 and height > 0 and mineCount > 0)
        {
            MainApp(width, height, mineCount)();

            return EXIT_SUCCESS;
        }
    }

    MainApp()();

    return EXIT_SUCCESS;
}
