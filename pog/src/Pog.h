#pragma once

#include <SFML/Graphics.hpp>
#include <array>

#define GAME_W 320
#define GAME_H 240
#define GAME_WF (float)GAME_W
#define GAME_HF (float)GAME_H
#define PADDLE_OFFSET 20.0f
#define SERVE_TIME 75.0f
#define BALL_ACC 0.25f
#define PADDLE_SPEED 5.0f
#define SERVE_SPEED 3.0f

struct Paddle
{
	sf::FloatRect rect() { return sf::FloatRect(pos - size / 2.0f, size); }

	sf::Vector2f pos;
	sf::Vector2f size{10.0f, 50.0f};
};

struct Ball
{
	sf::FloatRect rect() { return sf::FloatRect(pos - size / 2.0f, size); }

	sf::Vector2f pos;
	sf::Vector2f size{10.0f, 10.0f};
	sf::Vector2f vel;
};

class Pog
{
public:
	void run();
	void update();
	void render();
	void reset();

	sf::RenderWindow window;

	std::array<Paddle, 2> paddle;
	std::array<int, 2> score = {};
	Ball ball;
	float serve_timer = 0.0f;
	float serve_dir = 1.0f;
};
