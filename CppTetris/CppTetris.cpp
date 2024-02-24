#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <array>
#include <ctime>
#include <map>

// reformatting ideas:
// draw the shapes onto a separate texture before displaying it on the main window



const sf::Color FIELD_COLOUR = sf::Color({ 48, 39, 32 });
const short TILE_SIZE = 50.0f;
const short FIELD_BLOCK_W = 10, FIELD_BLOCK_H = 20;
short FIELD_RES[2] = { (FIELD_BLOCK_W + 5)*TILE_SIZE, FIELD_BLOCK_H * TILE_SIZE };
const float OUTLINE_THICKNESS_BLOCK = 3.0f;
const char ALL_TETROMINO_TYPE[7] = {'I', 'J', 'O', 'L', 'S', 'Z', 'T'};

sf::Color OUTLINE_COLOR_BLOCK = sf::Color::White;



typedef std::array<std::array<bool, FIELD_BLOCK_W>, FIELD_BLOCK_H> CollisionMap;

using TetrominoType = char;

struct TetrominoConfig {
    unsigned short x[4];
    unsigned short y[4];

};
std::map<TetrominoType, std::vector<TetrominoConfig>> TetrominoArr;
std::map<TetrominoType, sf::Color> TetrominoColor;

static void init_tetromino_settings() {
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    TetrominoArr['I'] = {
        {{0, 0, 0, 0}, {0, 1, 2, 3}}, // Horizontal
        {{0, 1, 2, 3}, {0, 0, 0, 0}}  // Vertical
    };

    TetrominoArr['J'] = {
        {{0, 0, 0, 1}, {0, 1, 2, 2}}, // Original
        {{0, 1, 2, 2}, {0, 0, 0, 1}}, // 90 
        {{0, 0, 0, 1}, {2, 0, 1, 2}}, // 180 
        {{0, 1, 2, 2}, {1, 1, 1, 0}}  // 270 
    };

    TetrominoArr['L'] = {
        {{0, 1, 2, 2}, {0, 0, 0, 1}}, // Original
        {{1, 1, 2, 1}, {1, 0, 0, 2}}, // 90 
        {{0, 0, 1, 2}, {0, 1, 1, 1}}, // 180
        {{0, 1, 1, 1}, {2, 2, 1, 0}}  // 270 
        // ... other rotations
    };

    TetrominoArr['O'] = {
        {{0, 1, 0, 1}, {0, 0, 1, 1}}
    };

    TetrominoArr['S'] = {
        {{0, 1, 1, 2}, {1, 1, 0, 0}}, // Original
        {{0, 0, 1, 1}, {0, 1, 1, 2}}, // 90 
        {{0, 1, 1, 2}, {1, 1, 0, 0}}, // 180 
        {{0, 0, 1, 1}, {0, 1, 1, 2}}  // 270 
        // ... other rotations
    };

    TetrominoArr['Z'] = {
        {{0, 1, 1, 2}, {0, 0, 1, 1}}, // Original
        {{0, 1, 1, 0}, {1, 0, 1, 2}}, // 90

    };

    TetrominoArr['T'] = {
        {{0, 1, 1, 2}, {1, 0, 1, 1}}, // Original
        {{2, 1, 1, 1}, {1, 0, 1, 2}}, // 90 
        {{2, 0, 1, 1}, {1, 1, 1, 2}}, // 180 
        {{1, 0, 1, 1}, {0, 1, 1, 2}}, // 270 
    };

    TetrominoColor['I'] = sf::Color::Blue;
    TetrominoColor['J'] = sf::Color{ 255, 0, 255 };
    TetrominoColor['O'] = sf::Color::Yellow;
    TetrominoColor['L'] = sf::Color::Green;
    TetrominoColor['T'] = sf::Color::Magenta;
    TetrominoColor['S'] = sf::Color::Red;
    TetrominoColor['Z'] = sf::Color{255, 255, 0};

}



class Block {

private:
    static std::vector<Block> *m_inactive_block_list;

    static std::vector<Block> m_get_block_to_clear(const std::vector<unsigned short> &row_to_clear) {
        std::vector<Block> block_to_clear = {};
        for (auto& b : *m_inactive_block_list) {
            if (std::find(row_to_clear.begin(), row_to_clear.end(), b.curr_pos.y) != row_to_clear.end()) {
                block_to_clear.push_back(b);
            }

        }
        return block_to_clear;
    }

    static void m_clear_block( std::vector<Block>& block_to_clear) {
        auto arr = m_inactive_block_list;

        arr->erase(std::remove_if(arr->begin(), arr->end(),
            [&block_to_clear](Block& element) {
                return std::find(block_to_clear.begin(), block_to_clear.end(), element) != block_to_clear.end();
            }),
            arr->end());



    }

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

    const std::vector<Block> get_block_list() {
        return *m_inactive_block_list;
    }

    void move(const sf::Vector2i& pos) {
        curr_pos.x += pos.x;
        curr_pos.y += pos.y;
        rect.setPosition(curr_pos.x * TILE_SIZE, curr_pos.y * TILE_SIZE);
    }

    void set(const sf::Vector2i& pos) {
        curr_pos.x = pos.x;
        curr_pos.y = pos.y;
        rect.setPosition(curr_pos.x * TILE_SIZE, curr_pos.y * TILE_SIZE);

    }

    static void set_inactive(std::array<Block, 4> b_arr) {
        m_inactive_block_list->insert(m_inactive_block_list->end(), b_arr.begin(), b_arr.end());
    }

    bool operator==(const Block& other) const {
        return (curr_pos.x == other.curr_pos.x) && (curr_pos.y == other.curr_pos.y);
    }

    static bool clear_row(const std::vector<unsigned short> &row_to_clear) {
        std::vector<Block> block_arr = m_get_block_to_clear(row_to_clear);
        if (block_arr.size() == 0) 
            return false;
        
        m_clear_block(block_arr);
        auto lowest_row_cleared_it = std::max_element(row_to_clear.begin(), row_to_clear.end());

        for (auto& b : *m_inactive_block_list) {
            if (b.curr_pos.y < *lowest_row_cleared_it) {
                b.move({ 0, 1 });
            }
        }

        return true;
    }

    static CollisionMap get_collision_map() {
        CollisionMap coll_map{};

        for (size_t i = 0; i < FIELD_BLOCK_H; i++) {
            coll_map[i] = {};
            for (size_t j = 0; j < FIELD_BLOCK_W; j++) {
                coll_map[i][j] = false;
            }
        }

        for (Block& b : *m_inactive_block_list) {
            auto& b_pos = b.curr_pos;
            coll_map[b_pos.y][b_pos.x] = true;
        }
        return coll_map;
    }

    static void reset() {
        m_inactive_block_list->clear();
    }



    static void draw(sf::RenderWindow &win) {
        for (Block& b : *m_inactive_block_list) {
            win.draw(b.rect);
        }
    }
};

std::vector<Block> *Block::m_inactive_block_list = new std::vector<Block>{};


class Tetromino {
private:
    std::vector<TetrominoConfig> m_all_state;
    unsigned short m_tot_state = 1;
    unsigned short m_curr_state = 0;

    std::array<Block, 4> m_block_arr;
    sf::Vector2i m_curr_pos;
    
    char m_shape;


public:

    sf::Vector2i m_past_move = { 0, 0 };

    Tetromino() {
        m_shape = ALL_TETROMINO_TYPE[std::rand() % 7];

        m_all_state = TetrominoArr.at(m_shape);
        m_tot_state = m_all_state.size();


        m_curr_pos = { FIELD_BLOCK_W / 2 - 1, 0 };
        TetrominoConfig cfg = m_all_state[m_curr_state];
        sf::Color b_color = TetrominoColor.at(m_shape);
        for (size_t i = 0; i < 4; i++) {
            m_block_arr[i] = Block(b_color, cfg.x[i] + FIELD_BLOCK_W / 2 - 1, cfg.y[i]);
        }

    }

    bool check_lost() {
        for (auto& b : m_block_arr) {
            if (b.curr_pos.y <=0) {
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderWindow &win) {
        for (Block& b : m_block_arr) {
            win.draw(b.rect);
        }
    }

    void move(const sf::Vector2i &pos) {
        for (Block& b : m_block_arr) {
            b.move(pos);
        }

        m_past_move = pos;
        m_curr_pos += pos;

    }

    void drop() {
        while (not check_collision()) {
            this->move({ 0, 1 });
        }

    }

    void rotate() {
        std::array<Block, 4> temp = m_block_arr;
        m_curr_state = (m_curr_state + 1) % m_tot_state;
        TetrominoConfig cfg = m_all_state[m_curr_state];
        for (size_t i = 0; i < 4; i++) {
            Block &b = m_block_arr[i];
            b.set({cfg.x[i] + m_curr_pos.x, cfg.y[i] + m_curr_pos.y});
        }

        if (check_collision())
            std::swap(m_block_arr, temp);


    }

    void resolve_collision() {
        move(-m_past_move);
    }

    void set_inactive() {
        Block::set_inactive(m_block_arr);
        
    }

    bool check_collision() {
        CollisionMap coll_map = Block::get_collision_map();
        for (Block& p : m_block_arr) {
            auto& p_pos = p.curr_pos;
            if (p_pos.x < 0 or (p_pos.x + 1) > FIELD_BLOCK_W or p_pos.y >= FIELD_BLOCK_H or coll_map[p_pos.y][p_pos.x])
                return true;

        }
        return false;


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


std::vector<unsigned short> check_row_to_clear(CollisionMap coll_map) {
    std::vector<unsigned short> row_num_arr = {};
    for (size_t row_num = 0; row_num < coll_map.size(); row_num++) {

        bool full = true;
        for (auto& empty : coll_map[row_num]) {
            if (not empty) {
                full = false;
                break;
            }

        }

        if (full)
            row_num_arr.push_back(row_num);
    }
    return row_num_arr;
}





class TextManager {
public:
    std::vector<std::unique_ptr<sf::Text>> text_list = {};
    sf::Font font;

    TextManager(std::string font_path) {
        if (!font.loadFromFile(font_path))
            std::cerr << "unable to load font .tff file" << std::endl;
    }


    sf::Text* get_text(std::string text, unsigned int size) {
        auto newText = std::make_unique<sf::Text>(text, font, size);
        sf::Text* ptr = newText.get();
        text_list.push_back(std::move(newText));
        return ptr;
    }

};

void draw(sf::RenderWindow& window, Tetromino& player) {
    draw_grid(window);
    player.draw(window);
    Block::draw(window);
}

int main() {

    init_tetromino_settings();
    sf::RenderWindow window(sf::VideoMode(FIELD_RES[0], FIELD_RES[1]), "CppTetris");
    window.setFramerateLimit(60);

    sf::Clock clock;
    float tot_elapsed = 0;

    Tetromino player{};

    int player_score = 0;
    bool position_changed = true;
    bool paused = false;
    bool lost = false;
    bool dropped = false;
       
    TextManager text_manager("C:/Users/yeapz/OneDrive/Desktop/C++/CppTetris/CppTetris/Fonts/Pixeboy.ttf");

    sf::Text* scoreboard = text_manager.get_text("Score: 0", 50);
    auto scoreboard_size = scoreboard->getGlobalBounds().width;
    scoreboard->setPosition(FIELD_RES[0] - (TILE_SIZE * 2.5 + scoreboard_size / 2), FIELD_RES[1] * 0.45);

    sf::Text* game_title = text_manager.get_text("TETRIS", 80);
    auto game_title_size = game_title->getGlobalBounds().width;
    game_title->setPosition(FIELD_RES[0] - (TILE_SIZE * 2.5 + game_title_size / 2), FIELD_RES[1] * 0.3);

    sf::Text* lose_screen = text_manager.get_text("You Lost, Try Again?", 40);
    auto lose_screen_size = lose_screen->getGlobalBounds().width;
    lose_screen->setPosition(FIELD_RES[0] - (TILE_SIZE * 2.5 + lose_screen_size / 2), FIELD_RES[1] * 0.5);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {

                switch (event.key.code) {
                case sf::Keyboard::Right:

                    player.move({ 1, 0 });
                    position_changed = true;
                    break;

                case sf::Keyboard::Left:
                    player.move({ -1, 0 });
                    position_changed = true;
                    break;

                case sf::Keyboard::Up:
                    player.rotate();
                    position_changed = true;
                    break;

                case sf::Keyboard::Down:
                    player.drop();
                    position_changed = true;
                    dropped = true;
                    break;

                case sf::Keyboard::Space:
                    paused = not paused;
                    if (lost) {
                        Block::reset();
                        player = Tetromino{};

                        position_changed = false;
                        dropped = false;
                        lost = false;

                    }
                    break;



                }


            }

        }
        sf::Time elapsed = clock.restart();

        if (paused) {
            continue;
        }
        tot_elapsed += elapsed.asMilliseconds();

        if (tot_elapsed >= 150 and not dropped) {
            tot_elapsed -= 150;
            player.move({ 0, 1 });
            position_changed = true;
        }



        if (position_changed and player.check_collision()) {
            player.resolve_collision();

            if (player.m_past_move.y == 0)
                continue;

            player.set_inactive();
            position_changed = false;
            dropped = false;

            std::vector<unsigned short> row_to_clear = check_row_to_clear(Block::get_collision_map());
            unsigned short num_row = row_to_clear.size();
            if (num_row >= 1) {
                Block::clear_row(row_to_clear);
                player_score += num_row;
                scoreboard->setString("Score: " + std::to_string(player_score));
            }

            else if (player.check_lost()) {
                scoreboard->setString("Score: 0");
                player_score = 0;
              
                paused = true;
                lost = true;
            }

            else {
                player = Tetromino{};
            }
        }
        window.clear();

        draw_grid(window);
        player.draw(window);
        Block::draw(window);
        window.draw(*scoreboard);
        window.draw(*game_title);
        if (lost)
            window.draw(*lose_screen);;


        window.display();
        }


    return 0;
    }

