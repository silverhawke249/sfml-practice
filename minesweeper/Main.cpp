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
sf::Color const BACKGROUND_COLOR {0x1B0345FF};
sf::Color const ALERT_COLOR {0x4A0202FF};

class MainApp
{
private:
    sf::RenderWindow window;

    GameBoard gameBoard;
    bool lmbHeld {false};
    float scalingFactor;
    float offsetX;
    float offsetY;
    sf::Transform boardTransform;

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

    void startNewGame(uint32_t boardWidth, uint32_t boardHeight, uint32_t mineCount)
    {
        // TODO: Modal popup to confirm if a game is ongoing
        this->gameBoard.initialize(boardWidth, boardHeight, mineCount);
        this->recalculateBoardPositioning();
    }

    void recalculateBoardPositioning()
    {
        auto [boardWidth, boardHeight] = this->gameBoard.getBoardDimensions();
        this->scalingFactor            = MAX_BOARD_SIZE / static_cast<float>(boardWidth);
        if (boardHeight * this->scalingFactor > MAX_BOARD_SIZE)
            this->scalingFactor = MAX_BOARD_SIZE / static_cast<float>(boardHeight);

        auto scaledWidth     = boardWidth * this->scalingFactor;
        auto scaledHeight    = boardHeight * this->scalingFactor;

        this->offsetX        = static_cast<float>(MAX_BOARD_SIZE - scaledWidth) / 2 + MARGIN;
        this->offsetY        = static_cast<float>(MAX_BOARD_SIZE - scaledHeight) / 2 + MARGIN + PANE_HEIGHT;

        this->boardTransform = sf::Transform {};
        this->boardTransform.translate(this->offsetX, this->offsetY)
            .scale(this->scalingFactor, this->scalingFactor, 0, 0);
    }

    void operator()()
    {
        recalculateBoardPositioning();

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
                        this->startNewGame(9, 9, 10);
                    if (ImGui::MenuItem("Intermediate (16x16)"))
                        this->startNewGame(16, 16, 40);
                    if (ImGui::MenuItem("Expert (30x16)"))
                        this->startNewGame(30, 16, 99);

                    ImGui::Separator();

                    if (ImGui::MenuItem("Custom..."))
                    {
                        // TODO: Modal popup to configure game settings
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            auto mousePos = sf::Mouse::getPosition(this->window);
            auto mX       = (mousePos.x - offsetX) / scalingFactor / TILE_SIZE;
            auto mY       = (mousePos.y - offsetY) / scalingFactor / TILE_SIZE;
            if (this->gameBoard.hasMine(mX, mY))
                this->window.clear(ALERT_COLOR);
            else
                this->window.clear(BACKGROUND_COLOR);

            this->window.draw(gameBoard, this->boardTransform);
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
