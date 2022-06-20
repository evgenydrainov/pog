#pragma once

#include <SFML/Graphics.hpp>
#include <array>

#define GAME_W 320
#define GAME_H 240
#define PADDLE_OFFSET 20
#define SERVE_TIME 75
#define BALL_ACC 1
#define PADDLE_SPEED 5
#define SERVE_SPEED 1

struct Paddle
{
	int x = 0;
	int y = 0;
	int width = 10;
	int height = 50;
};

struct Ball
{
	int x = 0;
	int y = 0;
	int hsp = 0;
	int vsp = 0;
	int width = 10;
	int height = 10;
};

class Pog
{
public:
	void run();
	void reset();

	sf::RenderWindow window;
	sf::Font font;

	std::array<Paddle, 2> paddles;
	std::array<int, 2> score = {};
	Ball ball;
	int serve_timer = 0;
	int serve_dir = 1;
};
