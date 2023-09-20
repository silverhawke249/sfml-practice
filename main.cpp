#include <cstdint>
#include <iostream>
#include <random>
#include <ranges>
#include <set>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

constexpr uint32_t WINDOW_WIDTH {1024};
constexpr uint32_t WINDOW_HEIGHT {768};
constexpr char const* WINDOW_TITLE {"Hello world!"};
constexpr uint32_t MARGIN {25};

class MainApp
{
private:
    sf::RenderWindow window;
    uint32_t windowWidth;
    uint32_t windowHeight;

    // Misc state
    bool lmbClicked {false};
    float tileWidth;
    float offsetX;
    float offsetY;

    // Game state
    uint32_t boardWidth;
    uint32_t boardHeight;
    uint32_t numTiles;
    std::set<std::tuple<uint32_t, uint32_t>> mineLocation;
    std::vector<bool> boardState;

public:
    MainApp(uint32_t windowWidth, uint32_t windowHeight, std::string_view windowTitle, uint32_t boardWidth = 16,
            uint32_t boardHeight = 16, uint32_t mineCount = 40):
        window(sf::VideoMode(windowWidth, windowHeight), windowTitle.data(), sf::Style::Default ^ sf::Style::Resize),
        windowWidth(windowWidth), windowHeight(windowHeight)
    {
        this->initializeBoard(boardWidth, boardHeight, mineCount);
    }

    void initializeBoard(uint32_t width, uint32_t height, uint32_t mineCount)
    {
        this->boardWidth  = width;
        this->boardHeight = height;
        this->numTiles    = width * height;
        this->mineLocation.clear();
        this->boardState.clear();
        if (mineCount > this->numTiles)
            mineCount = this->numTiles;

        this->tileWidth = (this->windowHeight - 2 * MARGIN) / static_cast<float>(this->boardHeight);
        auto gridWidth  = this->boardWidth * this->tileWidth;
        if (gridWidth > this->windowWidth - 2 * MARGIN)
            this->tileWidth = (this->windowWidth - 2 * MARGIN) / static_cast<float>(this->boardWidth);

        std::vector<uint32_t> possibleLocations(this->numTiles);
        std::iota(possibleLocations.begin(), possibleLocations.end(), 0);

        std::mt19937 rng {std::random_device {}()};
        std::shuffle(possibleLocations.begin(), possibleLocations.end(), rng);
        for (int i = 0; i < mineCount; ++i)
            this->mineLocation.emplace(possibleLocations[i] / width, possibleLocations[i] % width);

        this->boardState.resize(this->numTiles, false);
    }

    void drawBoard()
    {
        auto gridHeight = this->boardHeight * this->tileWidth;
        auto gridWidth  = this->boardWidth * this->tileWidth;

        this->offsetX   = MARGIN + (this->windowWidth - 2 * MARGIN - gridWidth) / 2;
        this->offsetY   = MARGIN + (this->windowHeight - 2 * MARGIN - gridHeight) / 2;

        for (auto const i: std::views::iota(0u, this->boardHeight))
            for (auto const j: std::views::iota(0u, this->boardWidth))
            {
                sf::RectangleShape rectangle {
                    sf::Vector2f {this->tileWidth, this->tileWidth}
                };
                if (this->boardState[j + i * this->boardWidth])
                    rectangle.setFillColor(sf::Color {0x404040FF});
                else
                    rectangle.setFillColor(sf::Color {(i + j) % 2 ? 0x808080FF : 0x707070FF});

                sf::Transform transform;
                transform.translate(this->offsetX + j * this->tileWidth, this->offsetY + i * this->tileWidth);

                this->window.draw(rectangle, transform);
            }
    }

    void handleClick(uint32_t xPos, uint32_t yPos)
    {
        auto boardX = (xPos - this->offsetX) / this->tileWidth;
        auto boardY = (yPos - this->offsetY) / this->tileWidth;
        if (boardX < 0 or boardX >= this->boardWidth or boardY < 0 or boardY >= this->boardHeight)
            return;

        auto tileX = static_cast<uint32_t>(boardX);
        auto tileY = static_cast<uint32_t>(boardY);

        this->boardState[tileX + tileY * this->boardWidth] = true;
    }

    void operator()()
    {
        sf::Event event;
        while (this->window.isOpen())
        {
            while (this->window.pollEvent(event))
                switch (event.type)
                {
                case sf::Event::Closed:
                    this->window.close();
                    break;
                case sf::Event::MouseButtonPressed:
                    this->lmbClicked = event.mouseButton.button == sf::Mouse::Left;
                    break;
                case sf::Event::MouseButtonReleased:
                    if (this->lmbClicked and event.mouseButton.button == sf::Mouse::Left)
                        this->handleClick(event.mouseButton.x, event.mouseButton.y);
                    this->lmbClicked = false;
                    break;
                default:
                    break;
                }

            this->window.clear(sf::Color::Black);

            this->drawBoard();

            this->window.display();
        }
    }
};

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        auto width  = std::stoi(argv[1]);
        auto height = std::stoi(argv[2]);
        if (width > 0 and height > 0)
        {
            MainApp(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, width, height)();

            return EXIT_SUCCESS;
        }
    }

    MainApp(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)();

    return EXIT_SUCCESS;
}
