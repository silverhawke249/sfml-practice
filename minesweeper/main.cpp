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

constexpr uint32_t WINDOW_WIDTH {600};
constexpr uint32_t WINDOW_HEIGHT {800};
constexpr char const* WINDOW_TITLE {"Minesweeper!"};
constexpr uint32_t MARGIN {25};
constexpr uint32_t BACKGROUND_COLOR {0x1B0345FF};

class MainApp
{
private:
    sf::RenderWindow window;
    uint32_t windowWidth;
    uint32_t windowHeight;

    GameBoard gameBoard;
    bool lmbHeld {false};

public:
    MainApp(uint32_t windowWidth, uint32_t windowHeight, std::string_view windowTitle, uint32_t boardWidth = 16,
            uint32_t boardHeight = 16, uint32_t mineCount = 40):
        window(sf::VideoMode(windowWidth, windowHeight), windowTitle.data(), sf::Style::Default ^ sf::Style::Resize),
        windowWidth(windowWidth), windowHeight(windowHeight), gameBoard(boardWidth, boardHeight, mineCount)
    {
        consoleLog("Initializing app...");
        std::ignore = ImGui::SFML::Init(this->window);
    }

    ~MainApp()
    {
        ImGui::SFML::Shutdown();
    }

    void operator()()
    {
        auto boardMaxWidth             = this->windowWidth - 2 * MARGIN;
        auto boardMaxHeight            = this->windowHeight - 2 * MARGIN;

        auto [boardWidth, boardHeight] = this->gameBoard.getBoardDimensions();
        float scalingFactor {boardMaxWidth / static_cast<float>(boardWidth)};
        if (boardHeight * scalingFactor > boardMaxHeight)
            scalingFactor = boardMaxHeight / static_cast<float>(boardHeight);

        auto scaledWidth  = boardWidth * scalingFactor;
        auto scaledHeight = boardHeight * scalingFactor;

        auto offsetX      = static_cast<float>(this->windowWidth - scaledWidth) / 2;
        auto offsetY      = static_cast<float>(this->windowHeight - scaledHeight) / 2;

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
            ImGui::ShowDemoWindow();

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
            MainApp(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, width, height, mineCount)();

            return EXIT_SUCCESS;
        }
    }

    MainApp(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)();

    return EXIT_SUCCESS;
}
