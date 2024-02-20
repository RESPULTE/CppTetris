#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <map>

// reformatting ideas:
// draw the shapes onto a separate texture before displaying it on the main window



const sf::Color FIELD_COLOUR = sf::Color({ 48, 39, 32 });
const short TILE_SIZE = 50.0f;
const short FIELD_BLOCK_W = 10, FIELD_BLOCK_H = 20;
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

    void move(const sf::Vector2i& pos) {
        curr_pos.x += pos.x;
        curr_pos.y += pos.y;
        rect.setPosition(curr_pos.x * TILE_SIZE, curr_pos.y * TILE_SIZE);
    }

    bool operator==(const Block& other) {
        return (curr_pos.x == other.curr_pos.x) && (curr_pos.y == other.curr_pos.y);
    }

};

class Tetromino {
private:
    std::vector<std::array<Block, 4>> m_block_arr;
    unsigned short m_curr_state = 0;
    unsigned short m_tot_state = 1;

public:
    char shape;
    sf::Vector2i m_past_move = {0, 0};

    Tetromino(char shape) : shape(shape) {
        sf::Color b_color = TetrominoColor.at(shape);
        std::vector<TetrominoConfig> tetro_config = TetrominoArr.at(shape);

        int mid_offset = FIELD_BLOCK_W / 2 - 1;
        for (size_t i = 0; i < tetro_config.size(); i++) {
            m_block_arr.push_back({});
            for (size_t j = 0; j < 4; j++) {
                TetrominoConfig cfg = tetro_config[i];
                m_block_arr[i][j] = Block(b_color, cfg.x[j] + mid_offset, cfg.y[j]);
            }
        }
        m_tot_state = tetro_config.size();


    }
    std::array<Block, 4> get_arr() {
        return m_block_arr[m_curr_state];
    }

    void draw(sf::RenderWindow &win) {
        for (Block &b : m_block_arr[m_curr_state]) {
            win.draw(b.rect);
        }
    }

    void move(const sf::Vector2i &pos) {
        for (Block &b : m_block_arr[m_curr_state]) {
            b.move(pos);
        }

        m_past_move = pos;

    }

    void rotate() {
        m_curr_state = (m_curr_state + 1) % m_tot_state;
    }

    void resolve_collision() {
        if (m_past_move.y > 0) {
            move({ 0, -1 });
            return;
        }

        if (m_past_move.x < 0) {
            move({ 1, 0 });
            return;
        }
        if (m_past_move.x > 0) {
            move({ -1, 0 });
            return;
        }
    }

    void print_pos() {
        for (Block& b : m_block_arr[m_curr_state]) {
            printf("(%d, %d)", b.curr_pos.x, b.curr_pos.y);
        }
        std::cout << '\n';
    }

};
void draw_grid(sf::RenderWindow& window) {
    sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    rect.setFillColor(FIELD_COLOUR);
    rect.setOutlineColor(sf::Color::Blue);
    rect.setOutlineThickness(3.0f);

    for (int i = 0; i < FIELD_BLOCK_W; i++) {
        for (int j = 0; j < FIELD_BLOCK_H; j++) {
            rect.setPosition(i * TILE_SIZE, j * TILE_SIZE);

            window.draw(rect);
        }
    }
}


void draw(sf::RenderWindow &window) {
    draw_grid(window);
}


bool check_collision(Tetromino &player, std::vector<Block> &block_list) {
    for (Block& p : player.get_arr()) {
        auto& p_pos = p.curr_pos;
        if (p_pos.x < 0 or (p_pos.x+1) > FIELD_BLOCK_W or p_pos.y >= FIELD_BLOCK_H) {
            return true;
        }
    }
    for (Block& b : block_list) {
        auto& b_pos = b.curr_pos;
        for (Block& p : player.get_arr()) {
            auto& p_pos = p.curr_pos;
            if (p_pos.x == b_pos.x and p_pos.y == b_pos.y) {
                return true;
            }
        }
    }

    return false;


}



void clear_row(std::vector<Block>& block_list) {
    std::map<unsigned short, std::array<bool, FIELD_BLOCK_W>> game_map;

    for (size_t i = 0; i < FIELD_BLOCK_H; i++) {
        game_map[i] = {};
        for (size_t j = 0; j < FIELD_BLOCK_W; j++) {
            game_map[i][j] = false;
        }
    }

    for (Block& b : block_list) {
        auto& b_pos = b.curr_pos;
        game_map[b_pos.y][b_pos.x] = true;
    }

    std::vector<unsigned short> row_num_arr = {};
    for (const auto& pair : game_map) {
        auto& row_num = pair.first;
        auto & row_arr = pair.second;

        bool not_full = false;
        for (auto& val : row_arr) {
            if (val != true) {
                not_full = true;
                break;
            }

        }

        if (not not_full) {
            row_num_arr.push_back(row_num);
        }
    }

    if (row_num_arr.size() < 1)
        return;
       
    
    std::vector<Block> block_to_clear = {};
    for (auto& b : block_list) {
        if (std::find(row_num_arr.begin(), row_num_arr.end(), b.curr_pos.y) != row_num_arr.end()) {
            block_to_clear.push_back(b);
        }


    }

    block_list.erase(std::remove_if(block_list.begin(), block_list.end(),
        [&block_to_clear](const Block& element) {
            return std::find(block_to_clear.begin(), block_to_clear.end(), element) != block_to_clear.end();
        }),
        block_list.end());

    for (auto& b : block_list) {
        b.move({ 0, 1 });
    }
}

int main() {
    init_tetromino_settings();
    sf::RenderWindow window(sf::VideoMode(FIELD_RES[0], FIELD_RES[1]), "CppTetris");
    window.setFramerateLimit(60);

    sf::Clock clock;
    float tot_elapsed = 0;

    Tetromino player{ 'J' };

    std::vector<Block> block_list = {};

    while (window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        tot_elapsed += elapsed.asMilliseconds();
        if (tot_elapsed >= 100) {
            tot_elapsed -= 100;
            player.move({ 0, 1 });
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right)
                    player.move({ 1, 0 });

                else if (event.key.code == sf::Keyboard::Left)
                    player.move({ -1, 0 });

                else if (event.key.code == sf::Keyboard::Space)
                    player.rotate();

            }

        }

        if (check_collision(player, block_list)) {
            player.resolve_collision();

            if (player.m_past_move.y == 0)
                continue;

            for (Block& b: player.get_arr())
                block_list.push_back(b);

            clear_row(block_list);


            player = Tetromino{ 'J' };
            if (check_collision(player, block_list)) {
                std::cout << "You lost";
                window.close();
            }
        }


        window.clear();
        draw_grid(window);
        player.draw(window);
        for (auto& block : block_list) {
            window.draw(block.rect);
        }
        window.display();
    }

    return 0;
}