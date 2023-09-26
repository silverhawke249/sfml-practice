#include "Minesweeper.h"

#include <numeric>
#include <queue>
#include <random>
#include <ranges>

sf::Sprite TextureManager::getSprite(SpriteType spriteType) const
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
    case SpriteType::INCORRECT_FLAG_TILE:
        sprite.setTextureRect({3 * TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE});
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

int32_t GameBoard::getMineNumber(std::tuple<int32_t, int32_t> coords) const
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

int32_t GameBoard::countFlags(int32_t x, int32_t y) const
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

bool GameBoard::checkWinCon() const
{
    uint32_t count = 0;
    for (auto s: this->boardState)
        count += s == TileState::COVERED or s == TileState::FLAGGED;

    return count == this->mineLocation.size();
}

bool GameBoard::checkLoseCon() const
{
    for (auto const& [x, y]: this->mineLocation)
        if (this->getBoardState(x, y) == TileState::UNCOVERED)
            return true;

    return false;
}

void GameBoard::floodFill(int32_t x, int32_t y)
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

void GameBoard::draw(sf::RenderTarget& target, sf::RenderStates states) const
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
                case GameState::GAME_NOT_STARTED:
                case GameState::GAME_ONGOING:
                    if (this->telegraphedTile.contains({x, y}))
                        sprite = this->textureMgr.getSprite(SpriteType::UNCOVERED_0);
                    else
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
                case GameState::GAME_NOT_STARTED:
                case GameState::GAME_ONGOING:
                case GameState::GAME_WON:
                    sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                    break;
                case GameState::GAME_LOST:
                    if (this->mineLocation.contains({x, y}))
                        sprite = this->textureMgr.getSprite(SpriteType::FLAGGED_TILE);
                    else
                        sprite = this->textureMgr.getSprite(SpriteType::INCORRECT_FLAG_TILE);
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

void GameBoard::initialize(int32_t boardWidth, int32_t boardHeight, int32_t mineCount)
{
    consoleLog("Initializing game board...");
    consoleLog("Board width = " + std::to_string(boardWidth));
    consoleLog("Board height = " + std::to_string(boardHeight));
    consoleLog("Mine count = " + std::to_string(mineCount));
    this->boardWidth  = boardWidth;
    this->boardHeight = boardHeight;
    this->mineCount   = mineCount;
    this->numTiles    = boardWidth * boardHeight;
    this->gameState   = GameState::GAME_NOT_STARTED;
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

void GameBoard::interact(float x, float y, sf::Mouse::Button mouseBtn)
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
        {
            this->gameState = GameState::GAME_ONGOING;
            this->setBoardState(x, y, TileState::UNCOVERED);
        }
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

    // Losing on first turn is not allowed
    if (this->gameState == GameState::GAME_NOT_STARTED and this->mineLocation.contains(this->lastClickedCoords))
    {
        consoleLog("Moving mine...");
        this->mineLocation.erase(this->lastClickedCoords);
        for (auto i: std::views::iota(0, this->numTiles))
        {
            if (this->mineLocation.contains(this->deflatten(i)))
                continue;
            if (this->deflatten(i) == this->lastClickedCoords)
                continue;
            this->mineLocation.emplace(this->deflatten(i));
            break;
        }
    }

    this->floodFill(x, y);

    if (this->checkLoseCon())
        this->gameState = GameState::GAME_LOST;
    else if (this->checkWinCon())
        this->gameState = GameState::GAME_WON;
}

void GameBoard::telegraph(float x, float y)
{
    if (this->isOutOfBounds(x, y))
        return;

    this->clearTelegraph();
    switch (this->getBoardState(x, y))
    {
    case TileState::COVERED:
        this->telegraphedTile.emplace(x, y);
        break;
    case TileState::UNCOVERED:
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
                    this->telegraphedTile.emplace(nX, nY);
            }
        break;
    case TileState::FLAGGED:
        break;
    }
}

void GameBoard::clearTelegraph()
{
    this->telegraphedTile.clear();
}
