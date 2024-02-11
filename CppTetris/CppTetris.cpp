#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <map>

// reformatting ideas:
// draw the shapes onto a separate texture before displaying it on the main window



const sf::Color FIELD_COLOUR = sf::Color({ 48, 39, 32 });
const short TILE_SIZE = 50;
short FIELD_BLOCK_W = 10, FIELD_BLOCK_H = 20;
short FIELD_RES[2] = {FIELD_BLOCK_W * TILE_SIZE, FIELD_BLOCK_H * TILE_SIZE};
float OUTLINE_THICKNESS_BLOCK = 3.0f;
sf::Color OUTLINE_COLOR_BLOCK = sf::Color::White;

// location for constituent blocks for each state 
// represented in a 4x4 grid, indexed from 0 to 15, left-to-right, up-to-down
typedef std::pair<unsigned short, unsigned short> Coordinate;
using TetrominoType = char;

struct TetrominoConfig {
    unsigned short x[4];
    unsigned short y[4];

};
std::map<TetrominoType, std::vector<TetrominoConfig>> TetrominoArr;
std::map<TetrominoType, sf::Color> TetrominoColor;

static void init_tetromino_settings() {
    TetrominoArr['I'] = {
        {{0, 0, 0, 0}, {0, 1, 2, 3}}, // Horizontal
        {{0, 1, 2, 3}, {0, 0, 0, 0}}  // Vertical
    };

    TetrominoArr['J'] = {
        {{0, 0, 0, 1}, {0, 1, 2, 2}},
        {{0, 1, 2, 2}, {0, 0, 0, 1}},
        // ... other rotations
    };

    TetrominoArr['O'] = {
        {{0,1,0,1}, {0, 0, 1, 1}}
    };

    TetrominoColor['I'] = sf::Color::Blue;
    TetrominoColor['J'] = sf::Color{ 255, 0, 255 };
    TetrominoColor['O'] = sf::Color::Yellow;
}


class Block {
public:
    sf::Vector2i curr_pos;
    sf::RectangleShape rect{ { TILE_SIZE, TILE_SIZE } };

    Block() = default;
    Block(sf::Color colour, unsigned short bx, unsigned short by) {
        curr_pos = { bx, by };
        rect.setFillColor(colour);
        rect.setOutlineColor(OUTLINE_COLOR_BLOCK);
        rect.setOutlineThickness(OUTLINE_THICKNESS_BLOCK);
        rect.setPosition((float)curr_pos.x * TILE_SIZE, (float)curr_pos.y * TILE_SIZE);
    }

    void move(sf::Vector2i pos) {
        curr_pos.x += pos.x;
        curr_pos.x += pos.x;
        rect.setPosition(curr_pos.x * TILE_SIZE, curr_pos.y * TILE_SIZE);

    }

};

class Tetromino {
private:
    std::vector<std::array<Block, 4>> arr;
    unsigned short curr_state = 0;

public:
    char shape;

    Tetromino(char shape) : shape(shape) {
        sf::Color b_color = TetrominoColor.at(shape);
        std::vector<TetrominoConfig> all_tetro_config = TetrominoArr.at(shape);
        for (size_t i = 0; i < all_tetro_config.size(); i++) {
            arr.push_back({});
            for (size_t j = 0; j < 4; j++) {
                auto cfg = all_tetro_config[i];
                arr[i][j] = Block(b_color, cfg.x[j], cfg.y[j]);
            }
        }

    }

    void draw(sf::RenderWindow *win) {
        for (Block b : arr[curr_state]) {
            win->draw(b.rect);
        }
    }

};

class TetrisApp {
public:
    sf::RenderWindow* window;
    TetrisApp(sf::RenderWindow* win) {
        window = win;
    }

    void update() {
        window->clear();
        draw();
        window->display();

    }

    void draw() {
        draw_grid();
    }

    void draw_grid() {
        sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        rect.setFillColor(FIELD_COLOUR);
        rect.setOutlineColor(sf::Color::Blue);
        rect.setOutlineThickness(3.0f);
        Tetromino test('O');

        for (int i = 0; i < FIELD_BLOCK_W; i++) {
            for (int j = 0; j < FIELD_BLOCK_H; j++) {
                rect.setPosition(i * TILE_SIZE, j * TILE_SIZE);

                window->draw(rect);
            }
        }
        test.draw(window);

    }
};



int main() {
    init_tetromino_settings();
    sf::RenderWindow window(sf::VideoMode(FIELD_RES[0], FIELD_RES[1]), "CppTetris");
    TetrisApp game(&window);



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

        }

        game.update();
    }

    return 0;
}