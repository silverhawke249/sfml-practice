#pragma once

#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

#include <SFML/Graphics.hpp>

constexpr uint32_t TILE_SIZE {64};
constexpr float TILE_SCALE {0.5};

constexpr int32_t DIGIT_WIDTH {64};
constexpr int32_t DIGIT_HEIGHT {128};

enum class TileState : uint8_t
{
    COVERED,
    UNCOVERED,
    FLAGGED,
};

enum class GameState : uint8_t
{
    GAME_NOT_STARTED,
    GAME_ONGOING,
    GAME_WON,
    GAME_LOST,
};

enum class SpriteType : uint8_t
{
    COVERED_TILE,
    FLAGGED_TILE,
    INCORRECT_FLAG_TILE,
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

enum class NumberValue : uint8_t
{
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    PERIOD,
};

inline void consoleLog(std::string_view message)
{
#ifdef DEBUG
    std::cout << message << std::endl;
#endif
}

class TextureManager
{
private:
    sf::Texture tileset;
    sf::Texture numbers;

public:
    TextureManager()
    {
        consoleLog("Initializing texture manager...");
        this->tileset.setSmooth(true);
    }

    void loadTextures()
    {
        consoleLog("Loading tileset...");
        this->tileset.loadFromMemory(minesweeper_tileset.data(), minesweeper_tileset.size());
        this->numbers.loadFromMemory(minesweeper_numbers.data(), minesweeper_numbers.size());
    }

    sf::Sprite getSprite(SpriteType spriteType) const;
    sf::Sprite getSprite(NumberValue digit) const;
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

    std::set<std::tuple<int32_t, int32_t>> telegraphedTile;

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

    int32_t getMineNumber(std::tuple<int32_t, int32_t> coords) const;
    int32_t countFlags(int32_t x, int32_t y) const;

    bool checkWinCon() const;
    bool checkLoseCon() const;

    void floodFill(int32_t x, int32_t y);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

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

    inline std::tuple<uint32_t, uint32_t> getBoardDimensions() const
    {
        return std::make_tuple(this->boardWidth * TILE_SIZE * TILE_SCALE,
                               this->boardHeight * TILE_SIZE * TILE_SCALE + DIGIT_HEIGHT);
    }

    inline auto getGameState() const
    {
        return this->gameState;
    }

    void initialize(int32_t boardWidth, int32_t boardHeight, int32_t mineCount);

    void interact(float x, float y, sf::Mouse::Button mouseBtn);
    void telegraph(float x, float y);
    void clearTelegraph();

    bool hasMine(float x, float y)
    {
#ifndef DEBUG
        return false;

#endif
        if (this->isOutOfBounds(x, y))
            return false;

        return this->mineLocation.contains({x, y});
    }
};
