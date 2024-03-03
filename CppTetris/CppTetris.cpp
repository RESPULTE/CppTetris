#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <vector>


#define LOG(x) std::cout << x << std::endl;



const sf::Color FIELD_COLOUR = sf::Color({ 48, 39, 32 });
const short TILE_SIZE = 50.0f;
const short FIELD_BLOCK_W = 10, FIELD_BLOCK_H = 22;
const short MENU_SIZE = 6;
short FIELD_RES[2] = { (FIELD_BLOCK_W + MENU_SIZE) * TILE_SIZE, (FIELD_BLOCK_H + 1) * TILE_SIZE };
const char ALL_TETROMINO_TYPE[7] = { 'I', 'J', 'O', 'L', 'S', 'Z', 'T' };

const unsigned short TOP_SCREEN_BLOCK_OFFSET = 2;
const unsigned short LEFT_SCREEN_BLOCK_OFFSET = 1;
const float OUTLINE_THICKNESS_BLOCK = 3.0f;

const short GAME_SPEED_INCREMENT = -25;
const unsigned short INITIAL_GAME_SPEED_MS = 300;
const unsigned short SCORE_TO_INCREMENT_GAME_SPEED = 5;


sf::Color OUTLINE_COLOR_BLOCK = sf::Color::White;

enum PlayerMove {
	Drop, Up, Down, Left, Right, Rotate, None
};

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

	TetrominoArr['L'] = {
		{{1, 1, 1, 0}, {0, 1, 2, 2}},  // 270 
		{{0, 1, 2, 2}, {0, 0, 0, 1}}, // 180 
		{{1, 1, 1, 2}, {0, 1, 2, 0}}, // Original
		{{0, 0, 1, 2}, {0, 1, 1, 1}}, // 90 
	};

	TetrominoArr['J'] = {
		{{1, 1, 1, 2}, {0, 1, 2, 2}}, // 90 
		{{0, 1, 2, 2}, {1, 1, 1, 0}}, // Original
		{{0, 1, 2, 0}, {1, 1, 1, 2}}, // 180
		{{0, 1, 1, 1}, {0, 2, 1, 0}},  // 270 
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

	TetrominoColor['I'] = sf::Color{ 0, 255, 255 };
	TetrominoColor['J'] = sf::Color{ 255, 165, 0 };
	TetrominoColor['O'] = sf::Color::Yellow;
	TetrominoColor['L'] = sf::Color::Blue;
	TetrominoColor['T'] = sf::Color::Magenta;
	TetrominoColor['S'] = sf::Color::Red;
	TetrominoColor['Z'] = sf::Color::Green;

}



class Block {

private:
	static std::vector<Block>* m_inactive_block_list;

	static std::vector<Block> m_get_block_to_clear(const std::vector<unsigned short>& row_to_clear) {
		std::vector<Block> block_to_clear = {};
		for (auto& b : *m_inactive_block_list) {
			if (std::find(row_to_clear.begin(), row_to_clear.end(), b.curr_pos.y) != row_to_clear.end()) {
				block_to_clear.push_back(b);
			}

		}
		LOG("'m_get_block_to_clear()' executed");
		return block_to_clear;
	}

	static void m_clear_block(std::vector<Block>& block_to_clear) {
		auto arr = m_inactive_block_list;

		arr->erase(std::remove_if(arr->begin(), arr->end(),
			[&block_to_clear](Block& element) {
				return std::find(block_to_clear.begin(), block_to_clear.end(), element) != block_to_clear.end();
			}),
			arr->end());
		LOG("'m_clear_block()' executed");



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
		rect.setPosition((float)(curr_pos.x + LEFT_SCREEN_BLOCK_OFFSET) * TILE_SIZE, (float)curr_pos.y * TILE_SIZE);

	}

	const std::vector<Block> get_block_list() {
		return *m_inactive_block_list;
	}

	void move(const sf::Vector2i& pos) {
		curr_pos.x += pos.x;
		curr_pos.y += pos.y;
		rect.setPosition((curr_pos.x + LEFT_SCREEN_BLOCK_OFFSET) * TILE_SIZE, curr_pos.y * TILE_SIZE);
	}

	void set(const sf::Vector2i& pos) {
		curr_pos.x = pos.x;
		curr_pos.y = pos.y;
		rect.setPosition((curr_pos.x + LEFT_SCREEN_BLOCK_OFFSET) * TILE_SIZE, curr_pos.y * TILE_SIZE);

	}

	static void set_inactive(std::array<Block, 4> b_arr) {
		m_inactive_block_list->insert(m_inactive_block_list->end(), b_arr.begin(), b_arr.end());
	}

	bool operator==(const Block& other) const {
		return (curr_pos.x == other.curr_pos.x) && (curr_pos.y == other.curr_pos.y);
	}

	static bool clear_row(const std::vector<unsigned short>& row_to_clear) {
		std::vector<Block> block_arr = m_get_block_to_clear(row_to_clear);

		m_clear_block(block_arr);

		int num_cleared_row = row_to_clear.size();
		for (auto& b : *m_inactive_block_list) {
			for (auto& row : row_to_clear) {
				if (b.curr_pos.y < row)
					b.move({ 0, num_cleared_row });
			}
		}

		LOG("'clear_row()' executed");

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



	static void draw(sf::RenderWindow& win) {
		for (Block& b : *m_inactive_block_list) {
			if (b.curr_pos.y >= TOP_SCREEN_BLOCK_OFFSET)
				win.draw(b.rect);
		}
	}
};

std::vector<Block>* Block::m_inactive_block_list = new std::vector<Block>{};

bool check_collision(std::array<Block, 4> blocks, CollisionMap coll_map) {
	for (Block& p : blocks) {
		auto& p_pos = p.curr_pos;
		if (p_pos.x < 0 or (p_pos.x + 1) > FIELD_BLOCK_W or p_pos.y >= FIELD_BLOCK_H or coll_map[p_pos.y][p_pos.x])
			return true;

	}
	return false;


}

class Tetromino {
private:
	std::vector<TetrominoConfig> m_all_state;
	unsigned short m_tot_state = 1;
	unsigned short m_curr_state = 0;

	std::array<Block, 4> m_block_arr;
	sf::Vector2i m_curr_pos;

	char m_shape;


public:

	PlayerMove m_past_move = None;


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

	const sf::Vector2i get_curr_pos() const {
		return m_curr_pos;
	}

	const std::array<Block, 4> get_block_arr() const {
		return m_block_arr;
	}

	void draw(sf::RenderWindow& win) {
		for (Block& b : m_block_arr) {
			win.draw(b.rect);
		}
	}

	bool move(const PlayerMove new_move) {
		sf::Vector2i move{ 0, 0 };

		set_past_move(new_move);
		switch (new_move) {
		case Up:
			move.y = -1;
			LOG("moved Up");

			break;

		case Down:
			move.y = 1;
			LOG("moved Down");

			break;

		case Left:
			move.x = -1;
			LOG("moved Left");

			break;

		case Right:
			move.x = 1;
			LOG("moved Right");

			break;

		case Drop:
			LOG("dropped");

			return drop();

		case Rotate:
			LOG("rotated");

			return rotate();
		}



		for (Block& b : m_block_arr) {
			b.move(move);
		}
		m_curr_pos += move;

		return check_collision(this->get_block_arr(), Block::get_collision_map());

	}

	const PlayerMove get_past_move() const {
		return m_past_move;
	}

	void set_past_move(PlayerMove move) {
		m_past_move = move;
	}

	bool drop() {
		while (not check_collision(this->get_block_arr(), Block::get_collision_map())) {
			this->move(Down);
		}
		return true;

	}

	bool rotate() {
		m_curr_state = (m_curr_state + 1) % m_tot_state;
		TetrominoConfig cfg = m_all_state[m_curr_state];
		for (size_t i = 0; i < 4; i++) {
			Block& b = m_block_arr[i];
			b.set({ cfg.x[i] + m_curr_pos.x, cfg.y[i] + m_curr_pos.y });
		}

		if (check_collision(this->get_block_arr(), Block::get_collision_map())) {
			return true;
		}
		return false;


	}

	void _rotate() {
		LOG("rotation reversed");
		auto new_state = m_curr_state - 1;
		// in case the state is negative
		new_state = (new_state >= 0) ? new_state : m_tot_state - 1;

		m_curr_state = (new_state) % m_tot_state;
		TetrominoConfig cfg = m_all_state[m_curr_state];
		for (size_t i = 0; i < 4; i++) {
			Block& b = m_block_arr[i];
			b.set({ cfg.x[i] + m_curr_pos.x, cfg.y[i] + m_curr_pos.y });
		}
	}

	void revert_past_state() {
		LOG("state revertion activated");

		switch (m_past_move)
		{
		case Drop:
			move(Up);

			break;

		case Down:
			move(Up);

			break;

		case Left:
			move(Right);

			break;

		case Right:
			move(Left);

			break;

		case Rotate:

			_rotate();

			break;

		}
	}


	void set_inactive() {
		Block::set_inactive(m_block_arr);

	}




};



bool check_lost(std::array<Block, 4> blocks) {
	for (auto& b : blocks) {
		if (b.curr_pos.y <= TOP_SCREEN_BLOCK_OFFSET) {
			return true;
		}
	}
	return false;
}

static void draw_grid(sf::RenderWindow& window) {
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setFillColor(FIELD_COLOUR);
	rect.setOutlineColor(sf::Color::Blue);
	rect.setOutlineThickness(3.0f);

	for (int i = LEFT_SCREEN_BLOCK_OFFSET; i < FIELD_BLOCK_W + LEFT_SCREEN_BLOCK_OFFSET; i++) {
		for (int j = TOP_SCREEN_BLOCK_OFFSET; j < FIELD_BLOCK_H; j++) {
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



int main() {

	init_tetromino_settings();
	sf::RenderWindow window(sf::VideoMode(FIELD_RES[0], FIELD_RES[1]), "CppTetris");
	window.setFramerateLimit(60);

	sf::Clock clock;
	float tot_elapsed = 0;

	Tetromino player{};
	PlayerMove player_move = None;
	bool cached_player_move = false;

	int player_score = 0;
	int block_speed_ms = INITIAL_GAME_SPEED_MS;
	bool paused = false;
	bool lost = false;
	bool collided = false;

	sf::Font font;
	font.loadFromFile("C:/Users/yeapz/OneDrive/Desktop/C++/CppTetris/CppTetris/Fonts/Pixeboy.ttf");

	sf::Text scoreboard("Score: 0", font, 50);
	auto scoreboard_size = scoreboard.getGlobalBounds().width;
	scoreboard.setPosition(FIELD_RES[0] - (static_cast<float>(TILE_SIZE * MENU_SIZE) / 2 + scoreboard_size / 2), FIELD_RES[1] * 0.45);

	sf::Text game_title("Tetris", font, 120);
	auto game_title_size = game_title.getGlobalBounds().width;
	game_title.setPosition(FIELD_RES[0] / 2 - game_title_size / 2, -40);

	sf::Text lose_screen("You Lost, Try Again?", font, 90);
	auto lose_screen_size = lose_screen.getGlobalBounds().width;
	lose_screen.setPosition(FIELD_RES[0] / 2 - lose_screen_size / 2, FIELD_RES[1] * 0.25);;

	sf::Text paused_screen("PAUSED", font, 200);
	auto paused_screen_size = paused_screen.getGlobalBounds().width;
	paused_screen.setPosition(FIELD_RES[0] / 2 - paused_screen_size / 2, FIELD_RES[1] * 0.25);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{

			if (cached_player_move) {
				collided = player.move(player_move);
				cached_player_move = false;
				player_move = None;
				LOG("executed cached move");
				break;
			}

			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::Right:
					player_move = Right;
					break;

				case sf::Keyboard::Left:
					player_move = Left;

					break;

				case sf::Keyboard::Up:
					player_move = Rotate;

					break;

				case sf::Keyboard::Down:
					player_move = Drop;

					break;

				case sf::Keyboard::Space:
					paused = not paused;

					if (lost) {
						Block::reset();
						player = Tetromino{};

						scoreboard.setString("Score: 0");
						player_score = 0;

						lost = false;
						LOG("new game started");
					}
					else
						LOG(((paused) ? "game paused" : "game resumed"));
					break;



				}

				if (paused)
					player_move = None;
			}

		}
		sf::Time elapsed = clock.restart();

		if (paused) {
			window.clear();

			draw_grid(window);

			if (player.get_curr_pos().y > 1)
				player.draw(window);
			Block::draw(window);
			window.draw(scoreboard);
			window.draw(game_title);

			if (lost) {
				window.draw(lose_screen);
			}

			else {
				window.draw(paused_screen);

			}

			window.display();
			continue;
		}

		tot_elapsed += elapsed.asMilliseconds();

		if (tot_elapsed >= block_speed_ms) {
			if (player_move != None) {
				cached_player_move = true;
				LOG("cached player move");
			}
			tot_elapsed -= block_speed_ms;
			collided = player.move(Down);
		}

		else if (player_move != None) {
			collided = player.move(player_move);
			player_move = None;
		}


		if (collided) {
			LOG("collision detected");

			collided = false;

			PlayerMove past_move = player.get_past_move();
			player.revert_past_state();
			if (past_move != Down) {
				continue;
			}
			player.set_inactive();
			LOG("player set inactive");

			std::vector<unsigned short> row_to_clear = check_row_to_clear(Block::get_collision_map());
			unsigned short num_row = row_to_clear.size();
			if (num_row >= 1) {
				LOG("rows detected to clear: " + std::to_string(num_row));
				Block::clear_row(row_to_clear);
				LOG("rows cleared");

				player_score += num_row;
				scoreboard.setString("Score: " + std::to_string(player_score));
				if (player_score % 5 == 0) {
					block_speed_ms -= GAME_SPEED_INCREMENT;
					LOG("game speed increased");
				}
				player = Tetromino{};
			}

			else if (check_lost(player.get_block_arr())) {
				LOG("player lost");


				paused = true;
				lost = true;
			}

			else {
				player = Tetromino{};
			}
		}
		window.clear();


		draw_grid(window);
		if (player.get_curr_pos().y > 1)
			player.draw(window);
		Block::draw(window);
		window.draw(scoreboard);
		window.draw(game_title);

		window.display();
	}



	return 0;
}

