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

// TODO: Store high scores

constexpr uint32_t BASE_SIZE {600};
constexpr uint32_t WINDOW_WIDTH {BASE_SIZE};
constexpr uint32_t WINDOW_HEIGHT {BASE_SIZE};
constexpr float UI_SCALE {0.5};
constexpr char const* WINDOW_TITLE {"Minesweeper!"};
sf::Color const BACKGROUND_COLOR {0xE0E0E0FF};
sf::Color const ALERT_COLOR {0x4A0202FF};

class GameType
{
private:
    int32_t boardWidth;
    int32_t boardHeight;
    int32_t mineCount;

public:
    GameType(int32_t boardWidth, int32_t boardHeight, int32_t mineCount):
        boardWidth(boardWidth), boardHeight(boardHeight), mineCount(mineCount)
    {
    }

    inline auto getBoardWidth() const
    {
        return this->boardWidth;
    }

    inline auto getBoardHeight() const
    {
        return this->boardHeight;
    }

    inline auto getMineCount() const
    {
        return this->mineCount;
    }
};

struct GameTypeLT
{
    bool operator()(GameType const& lhs, GameType const& rhs)
    {
        if ((lhs.getBoardWidth() <=> rhs.getBoardWidth()) != 0)
            return (lhs.getBoardWidth() <=> rhs.getBoardWidth()) < 0;
        if ((lhs.getBoardHeight() <=> rhs.getBoardHeight()) != 0)
            return (lhs.getBoardHeight() <=> rhs.getBoardHeight()) < 0;
        return lhs.getMineCount() < rhs.getMineCount();
    }
};

class HighScoreManager
{
private:
    std::map<GameType, int32_t, GameTypeLT> clickTable;
    std::map<GameType, int32_t, GameTypeLT> timeTable;

    HighScoreManager()
    {
        // Load stuff here
    }

public:
    HighScoreManager(HighScoreManager const&) = delete;
    void operator=(HighScoreManager const&)   = delete;

    static auto& getInstance()
    {
        static HighScoreManager instance;

        return instance;
    }
};

class MainApp
{
private:
    sf::RenderWindow window;

    GameBoard gameBoard;
    bool firstRun {true};
    bool debugAssist {false};
    bool lmbHeld {false};

    float scaleX {1.0};
    float scaleY {1.0};
    float offsetX;
    float offsetY;
    float menuBarHeight {0.0};

    sf::Transform boardTransform;

    inline float relativeToBoardX(int32_t pos) const
    {
        return (pos - this->offsetX) / TILE_SIZE / UI_SCALE;
    }

    inline float relativeToBoardY(int32_t pos) const
    {
        return (pos - this->offsetY) / TILE_SIZE / UI_SCALE;
    }

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
        this->resizeWindow();
    }

    void resizeWindow()
    {
        auto [boardWidth, boardHeight]    = this->gameBoard.getDrawableSize();
        auto [boardOffsetX, boardOffsetY] = this->gameBoard.getBoardOffset();
        // Okay so this is really dumb. When you resize the window, everything
        //   drawn assumes that the window has its original window size! So
        //   things get stretched around and stuff... you're gonna need to
        //   transform things drawn to counter that scaling.
        // ... HOWEVER! Mouse input position are unaffected, so be careful
        //   with that lmao
        // If you can, don't resize the window -- keep it the same size. Create
        //   a view instead and tinker around with that.
        this->window.setSize({static_cast<uint32_t>(UI_SCALE * boardWidth),
                              static_cast<uint32_t>(UI_SCALE * boardHeight + this->menuBarHeight)});

        this->scaleX         = static_cast<float>(WINDOW_WIDTH) / boardWidth;
        this->scaleY         = (static_cast<float>(WINDOW_HEIGHT) - this->menuBarHeight) / boardHeight;
        this->offsetX        = UI_SCALE * boardOffsetX;
        this->offsetY        = UI_SCALE * boardOffsetY + this->menuBarHeight;

        this->boardTransform = sf::Transform {};
        this->boardTransform.translate(0, 0 + this->menuBarHeight).scale({this->scaleX, this->scaleY});
    }

    void operator()()
    {
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
                auto imguiKeyCap   = ImGui::GetIO().WantCaptureKeyboard;

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
                        boardX        = this->relativeToBoardX(event.mouseButton.x);
                        boardY        = this->relativeToBoardY(event.mouseButton.y);
                        this->gameBoard.telegraph(boardX, boardY);
                    }
                    break;
                case sf::Event::MouseMoved:
                    if (imguiMouseCap)
                        continue;
                    if (this->lmbHeld)
                    {
                        boardX = this->relativeToBoardX(event.mouseMove.x);
                        boardY = this->relativeToBoardY(event.mouseMove.y);
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
                        boardX = this->relativeToBoardX(event.mouseButton.x);
                        boardY = this->relativeToBoardY(event.mouseButton.y);
                        this->gameBoard.interact(boardX, boardY, event.mouseButton.button);
                        break;
                    case GameState::GAME_WON:
                    case GameState::GAME_LOST:
                        this->gameBoard.initialize();
                        break;
                    }
                    break;
                case sf::Event::KeyReleased:
                    if (imguiKeyCap)
                        continue;
                    if (event.key.code == sf::Keyboard::Key::F12)
                        this->debugAssist = !this->debugAssist;
                default:
                    break;
                }
            }

            ImGui::SFML::Update(this->window, deltaClock.restart());

            if (ImGui::BeginMainMenuBar())
            {
                this->menuBarHeight = ImGui::GetWindowSize().y;

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

            if (this->firstRun)
            {
                this->firstRun = false;
                resizeWindow();
            }

            auto mousePos = sf::Mouse::getPosition(this->window);
            auto mX       = this->relativeToBoardX(mousePos.x);
            auto mY       = this->relativeToBoardY(mousePos.y);
            if (this->debugAssist and this->gameBoard.hasMine(mX, mY))
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
