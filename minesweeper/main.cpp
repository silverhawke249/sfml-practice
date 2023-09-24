#include <cstdint>
#include <filesystem>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

constexpr uint32_t WINDOW_WIDTH {1024};
constexpr uint32_t WINDOW_HEIGHT {768};
constexpr char const* WINDOW_TITLE {"Minesweeper!"};
constexpr uint32_t MARGIN {25};
constexpr uint32_t TILE_SIZE {64};
constexpr uint32_t BACKGROUND_COLOR {0x1B0345FF};

enum class TileState : uint8_t
{
    COVERED,
    UNCOVERED,
    FLAGGED,
};

enum class GameState : uint8_t
{
    GAME_ONGOING,
    GAME_WON,
    GAME_LOST,
};

enum class SpriteType : uint8_t
{
    COVERED_TILE,
    FLAGGED_TILE,
    INERT_MINE,
    DETONATED_MINE,
    UNCOVERED_0,
    UNCOVERED_1,
    UNCOVERED_2,
    UNCOVERED_3,
    UNCOVERED_4,
    UNCOVERED_5,
    UNCOVERED_6,
    UNCOVERED_7,
    UNCOVERED_8,
};

void consoleLog(std::string_view message)
{
    std::cout << message << std::endl;
}

class TextureManager
{
private:
    sf::Texture tileset;

public:
    TextureManager()
    {
        consoleLog("Initializing texture manager...");
    }

    void loadTextures()
    {
        consoleLog("Loading tileset...");
        this->tileset.loadFromFile("resources/tileset.png");
    }

    sf::Sprite getSprite(SpriteType spriteType) const
    {
        sf::Sprite sprite {this->tileset};
        switch (spriteType)
        {
        case SpriteType::COVERED_TILE:
            sprite.setTextureRect({0, 0, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::FLAGGED_TILE:
            sprite.setTextureRect({TILE_SIZE, 0, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::INERT_MINE:
            sprite.setTextureRect({2 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::DETONATED_MINE:
            sprite.setTextureRect({3 * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_0:
            sprite.setTextureRect({0, TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_1:
            sprite.setTextureRect({TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_2:
            sprite.setTextureRect({2 * TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_3:
            sprite.setTextureRect({0, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_4:
            sprite.setTextureRect({TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_5:
            sprite.setTextureRect({2 * TILE_SIZE, 2 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_6:
            sprite.setTextureRect({0, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_7:
            sprite.setTextureRect({TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        case SpriteType::UNCOVERED_8:
            sprite.setTextureRect({2 * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE});
            break;
        }

        return sprite;
    }
};

class GameBoard : public sf::Drawable
{
private:
    int32_t boardWidth;
    int32_t boardHeight;
    int32_t mineCount;

    TextureManager textureMgr;

    int32_t numTiles;
    GameState gameState;
    std::set<std::tuple<int32_t, int32_t>> mineLocation;
    std::vector<int32_t> mineCounts;
    std::vector<TileState> boardState;
    std::tuple<int32_t, int32_t> lastClickedCoords;

    inline auto flatten(int32_t x, int32_t y) const
    {
        return x + y * this->boardWidth;
    }

    inline auto deflatten(int32_t index) const
    {
        return std::make_tuple(index % this->boardWidth, index / this->boardWidth);
    }

    inline auto getMineCount(int32_t x, int32_t y) const
    {
        return this->mineCounts[flatten(x, y)];
    }

    inline auto setMineCount(int32_t x, int32_t y, int32_t value)
    {
        this->mineCounts[flatten(x, y)] = value;
    }

    inline auto getBoardState(int32_t x, int32_t y) const
    {
        return this->boardState[flatten(x, y)];
    }

    inline auto setBoardState(int32_t x, int32_t y, TileState value)
    {
        this->boardState[flatten(x, y)] = value;
    }

    inline auto isOutOfBounds(float x, float y) const
    {
        return x < 0 or x >= this->boardWidth or y < 0 or y >= this->boardHeight;
    }

    int32_t getMineNumber(std::tuple<int32_t, int32_t> coords) const
    {
        auto [x, y]   = coords;
        int32_t count = 0;
        for (auto i: std::views::iota(-1, 2))
            for (auto j: std::views::iota(-1, 2))
            {
                if (i == 0 and j == 0)
                    continue;
                if (this->mineLocation.contains({x + i, y + j}))
                    count++;
            }

        return count;
    }

    int32_t countFlags(int32_t x, int32_t y) const
    {
        int32_t count = 0;
        for (auto i: std::views::iota(-1, 2))
            for (auto j: std::views::iota(-1, 2))
            {
                if (i == 0 and j == 0)
                    continue;
                if (this->isOutOfBounds(x + i, y + j))
                    continue;
                if (this->getBoardState(x + i, y + j) == TileState::FLAGGED)
                    count++;
            }

        return count;
    }

    bool checkWinCon() const
    {
        int32_t count = 0;
        for (auto s: this->boardState)
            count += s == TileState::COVERED or s == TileState::FLAGGED;

        return count == this->mineLocation.size();
    }

    bool checkLoseCon() const
    {
        for (auto const& [x, y]: this->mineLocation)
            if (this->getBoardState(x, y) == TileState::UNCOVERED)
                return true;

        return false;
    }

    void floodFill(int32_t x, int32_t y)
    {
        std::queue<std::tuple<int32_t, int32_t>> tilesToCheck;
        if (this->getBoardState(x, y) != TileState::UNCOVERED)
            return;
        if (this->mineLocation.contains({x, y}))
            return;
        if (this->getMineCount(x, y) != 0)
            return;

        tilesToCheck.emplace(x, y);
        while (!tilesToCheck.empty())
        {
            auto [tileX, tileY] = tilesToCheck.front();
            for (auto i: std::views::iota(-1, 2))
                for (auto j: std::views::iota(-1, 2))
                {
                    if (i == 0 and j == 0)
                        continue;
                    auto nX = tileX + i;
                    auto nY = tileY + j;
                    // Out of bounds
                    if (isOutOfBounds(nX, nY))
                        continue;
                    // Not opened, not marked
                    if (this->getBoardState(nX, nY) == TileState::COVERED)
                    {
                        this->setBoardState(nX, nY, TileState::UNCOVERED);
                        if (this->getMineCount(nX, nY) == 0)
                            tilesToCheck.emplace(nX, nY);
                    }
                }

            tilesToCheck.pop();
        }
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Copy base transform from render state
        sf::Transform baseTransform {states.transform};

        for (auto y: std::views::iota(0, this->boardHeight))
            for (auto x: std::views::iota(0, this->boardWidth))
            {
                sf::Sprite sprite;
                switch (this->getBoardState(x, y))
                {
                case TileState::COVERED:
                    switch (this->gameState)
                    {
                    case GameState::GAME_ONGOING:
                        sprite = this->textureMgr.getSprite(SpriteType::COVERED_TILE);
                        break;
                    case GameState::GAME_WON:
                        sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                        break;
                    case GameState::GAME_LOST:
                        if (this->mineLocation.contains({x, y}))
                            sprite = this->textureMgr.getSprite(SpriteType::INERT_MINE);
                        else
                            sprite = this->textureMgr.getSprite(SpriteType::COVERED_TILE);
                        break;
                    }
                    break;
                case TileState::UNCOVERED:
                    if (this->mineLocation.contains({x, y}))
                    {
                        if (this->lastClickedCoords == std::tie(x, y))
                            sprite = this->textureMgr.getSprite(SpriteType::DETONATED_MINE);
                        else
                            sprite = this->textureMgr.getSprite(SpriteType::INERT_MINE);
                    }
                    else
                        switch (this->getMineCount(x, y))
                        {
                        case 1:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_1);
                            break;
                        case 2:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_2);
                            break;
                        case 3:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_3);
                            break;
                        case 4:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_4);
                            break;
                        case 5:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_5);
                            break;
                        case 6:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_6);
                            break;
                        case 7:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_7);
                            break;
                        case 8:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_8);
                            break;
                        default:
                            sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_0);
                            break;
                        }
                    break;
                case TileState::FLAGGED:
                    switch (this->gameState)
                    {
                    case GameState::GAME_ONGOING:
                    case GameState::GAME_WON:
                        sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                        break;
                    case GameState::GAME_LOST:
                        if (this->mineLocation.contains({x, y}))
                            sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                        else
                            // TODO: Make incorrect flag sprite
                            sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                        break;
                    }
                    break;
                }

                sf::Transform translate;
                translate.translate(x * TILE_SIZE, y * TILE_SIZE);
                states.transform = baseTransform * translate;

                target.draw(sprite, states);
            }
    }

public:
    GameBoard(int32_t boardWidth, int32_t boardHeight, int32_t mineCount): textureMgr()
    {
        this->textureMgr.loadTextures();

        this->initialize(boardWidth, boardHeight, mineCount);
    }

    void initialize()
    {
        this->initialize(this->boardWidth, this->boardHeight, this->mineCount);
    }

    void initialize(int32_t boardWidth, int32_t boardHeight, int32_t mineCount)
    {
        consoleLog("Initializing game board...");
        consoleLog("Board width = " + std::to_string(boardWidth));
        consoleLog("Board height = " + std::to_string(boardHeight));
        consoleLog("Mine count = " + std::to_string(mineCount));
        this->boardWidth  = boardWidth;
        this->boardHeight = boardHeight;
        this->mineCount   = mineCount;
        this->numTiles    = boardWidth * boardHeight;
        this->gameState   = GameState::GAME_ONGOING;
        this->mineLocation.clear();
        this->mineCounts.clear();
        this->boardState.clear();

        if (mineCount >= this->numTiles)
            this->mineCount = this->numTiles - 1;

        std::vector<uint32_t> possibleLocations(this->numTiles);
        std::iota(possibleLocations.begin(), possibleLocations.end(), 0);

        consoleLog("Placing mines...");
        std::mt19937 rng {std::random_device {}()};
        std::shuffle(possibleLocations.begin(), possibleLocations.end(), rng);
        for (int i = 0; i < mineCount; ++i)
            this->mineLocation.emplace(this->deflatten(possibleLocations[i]));

        consoleLog("Calculating tile contents...");
        this->mineCounts.resize(numTiles, 0);
        for (int i = 0; i < this->numTiles; ++i)
            this->mineCounts[i] = this->getMineNumber(this->deflatten(i));

        consoleLog("Populating board state...");
        this->boardState.resize(this->numTiles, TileState::COVERED);
    }

    inline std::tuple<int32_t, int32_t> getBoardDimensions() const
    {
        return std::make_tuple(this->boardWidth * TILE_SIZE, this->boardHeight * TILE_SIZE);
    }

    inline auto getGameState() const
    {
        return this->gameState;
    }

    void interact(float x, float y, sf::Mouse::Button mouseBtn)
    {
        if (mouseBtn != sf::Mouse::Button::Left and mouseBtn != sf::Mouse::Button::Right)
            return;
        if (this->isOutOfBounds(x, y))
            return;

        this->lastClickedCoords = std::make_tuple(x, y);
        switch (this->getBoardState(x, y))
        {
        case TileState::COVERED:
            if (mouseBtn == sf::Mouse::Button::Left)
                this->setBoardState(x, y, TileState::UNCOVERED);
            else
                this->setBoardState(x, y, TileState::FLAGGED);
            break;
        case TileState::UNCOVERED:
            if (mouseBtn == sf::Mouse::Button::Left)
            {
                if (this->getMineCount(x, y) != this->countFlags(x, y))
                    return;

                for (auto i: std::views::iota(-1, 2))
                    for (auto j: std::views::iota(-1, 2))
                    {
                        if (i == 0 and j == 0)
                            continue;
                        auto nX = x + i;
                        auto nY = y + j;
                        // Out of bounds
                        if (this->isOutOfBounds(nX, nY))
                            continue;
                        // Not opened, not marked
                        if (this->getBoardState(nX, nY) == TileState::COVERED)
                        {
                            this->setBoardState(nX, nY, TileState::UNCOVERED);
                            if (this->getMineCount(nX, nY) == 0)
                                this->floodFill(nX, nY);
                        }
                    }
            }
            else
                return;
            break;
        case TileState::FLAGGED:
            if (mouseBtn == sf::Mouse::Button::Left)
                return;
            else
                this->setBoardState(x, y, TileState::COVERED);
            break;
        }

        this->floodFill(x, y);

        if (this->checkLoseCon())
            this->gameState = GameState::GAME_LOST;
        else if (this->checkWinCon())
            this->gameState = GameState::GAME_WON;
    }
};

class MainApp
{
private:
    sf::RenderWindow window;
    uint32_t windowWidth;
    uint32_t windowHeight;

    GameBoard gameBoard;

public:
    MainApp(uint32_t windowWidth, uint32_t windowHeight, std::string_view windowTitle, uint32_t boardWidth = 16,
            uint32_t boardHeight = 16, uint32_t mineCount = 40):
        window(sf::VideoMode(windowWidth, windowHeight), windowTitle.data(), sf::Style::Default ^ sf::Style::Resize),
        windowWidth(windowWidth), windowHeight(windowHeight), gameBoard(boardWidth, boardHeight, mineCount)
    {
        consoleLog("Initializing app...");
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
        sf::Event event;
        float boardX;
        float boardY;
        while (this->window.isOpen())
        {
            while (this->window.pollEvent(event))
                switch (event.type)
                {
                case sf::Event::Closed:
                    this->window.close();
                    break;
                case sf::Event::MouseButtonReleased:
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

            this->window.clear(sf::Color(BACKGROUND_COLOR));

            this->window.draw(gameBoard, transform);

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
