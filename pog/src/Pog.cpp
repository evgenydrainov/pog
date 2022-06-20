#include "Pog.h"

static int clamp(int x, int min, int max) {
	return std::min(std::max(x, min), max);
}

static int sign(int x) {
	if (x > 0) {
		return 1;
	} else if (x == 0) {
		return 0;
	} else {
		return -1;
	}
}

static bool ball_collides_with_paddle(Ball& ball, Paddle& paddle) {
	sf::IntRect ball_rect;
	ball_rect.left = ball.x - ball.width / 2;
	ball_rect.top = ball.y - ball.height / 2;
	ball_rect.width = ball.width;
	ball_rect.height = ball.height;

	sf::IntRect paddle_rect;
	paddle_rect.left = paddle.x - paddle.width / 2;
	paddle_rect.top = paddle.y - paddle.height / 2;
	paddle_rect.width = paddle.width;
	paddle_rect.height = paddle.height;

	return ball_rect.intersects(paddle_rect);
}

static void draw_paddle(sf::RenderTarget& target, Paddle& paddle) {
	sf::RectangleShape r;
	r.setSize(sf::Vector2f(paddle.width, paddle.height));
	r.setPosition(paddle.x, paddle.y);
	r.setOrigin(paddle.width / 2, paddle.height / 2);
	target.draw(r);
}

static void draw_ball(sf::RenderTarget& target, Ball& ball) {
	sf::RectangleShape r;
	r.setSize(sf::Vector2f(ball.width, ball.height));
	r.setPosition(ball.x, ball.y);
	r.setOrigin(ball.width / 2, ball.height / 2);
	target.draw(r);
}

void Pog::run() {
	window.create(sf::VideoMode(GAME_W, GAME_H), "pog");
	window.setFramerateLimit(50);
	window.setKeyRepeatEnabled(false);

	font.loadFromFile("bit5x3.ttf");
	font.setSmooth(false);

	paddles[1].x = GAME_W - PADDLE_OFFSET;
	paddles[1].y = GAME_H / 2;

	paddles[0].x = PADDLE_OFFSET;
	paddles[0].y = GAME_H / 2;

	reset();

	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.hasFocus() ? window.pollEvent(event) : window.waitEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		sf::Time time = clock.restart();
		float delta = time.asSeconds();

		{
			sf::Vector2i screen = sf::Mouse::getPosition(window);
			sf::Vector2f world = window.mapPixelToCoords(screen);
			paddles[1].y = world.y;

			if (ball.y < paddles[0].y) paddles[0].y -= PADDLE_SPEED;
			if (ball.y > paddles[0].y) paddles[0].y += PADDLE_SPEED;

			for (Paddle& p : paddles) p.y = clamp(p.y, p.height / 2, GAME_H - p.height / 2);

			ball.x += ball.hsp;
			ball.y += ball.vsp;

			for (size_t i = 0; i < paddles.size(); i++) {
				if (ball_collides_with_paddle(ball, paddles[i])) {
					int dy = ball.y - paddles[i].y;
					if (i == 0) {
						ball.hsp = abs(ball.hsp) + BALL_ACC;
					} else {
						ball.hsp = -(abs(ball.hsp) + BALL_ACC);
					}
					ball.vsp = dy / 5;
					if (ball.vsp == 0) {
						ball.vsp = (rand() % 2) ? 1 : -1;
					}
					ball.vsp += sign(ball.vsp) * (abs(ball.hsp) / 3);
				}
			}

			if (ball.y < 0 || ball.y > GAME_H) {
				ball.vsp = -ball.vsp;
			}

			if (ball.x < 0) {
				reset();
				score[1]++;
				serve_dir = -1;
			}

			if (ball.x > GAME_W) {
				reset();
				score[0]++;
				serve_dir = 1;
			}

			if (serve_timer > 0) {
				serve_timer--;
				if (serve_timer <= 0) {
					ball.hsp = SERVE_SPEED * serve_dir;
				}
			}
		}

		window.clear();
		{
			for (Paddle& p : paddles) draw_paddle(window, p);

			draw_ball(window, ball);

			for (int y = 0; y < GAME_H; y += 10) {
				sf::RectangleShape r;
				r.setSize(sf::Vector2f(5, 5));
				r.setOrigin(2, 2);
				r.setPosition(GAME_W / 2, y);
				window.draw(r);
			}

			for (size_t i = 0; i < score.size(); i++) {
				sf::Text t;
				t.setFont(font);
				t.setCharacterSize(32);
				t.setString(std::to_string(score[i]));
				t.setPosition(GAME_W / 4 + (GAME_W / 2) * i, GAME_H / 10);
				window.draw(t);
			}
		}
		window.display();
	}
}

void Pog::reset() {
	ball.x = GAME_W / 2;
	ball.y = GAME_H / 2;

	ball.hsp = 0;
	ball.vsp = 0;

	serve_timer = SERVE_TIME;
}
