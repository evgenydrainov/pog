#include "Pog.h"

#include "EasyText.h"

static float clamp(float x, float min, float max) {
	return std::min(std::max(x, min), max);
}

static float approach(float start, float end, float shift) {
	return start + clamp(end - start, -shift, shift);
}

float length(sf::Vector2f v) {
	return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(sf::Vector2f v) {
	return v / length(v);
}

static void draw_rect(sf::RenderTarget& target, sf::FloatRect rect, sf::Color color) {
	sf::RectangleShape r;
	r.setPosition(rect.getPosition());
	r.setSize(rect.getSize());
	r.setFillColor(color);
	target.draw(r);
}

bool chance(int percent) {
	return (rand() % 100) < percent;
}

void Pog::run() {
	srand(time(nullptr));

	window.create(sf::VideoMode(GAME_W, GAME_H), "pog");
	window.setFramerateLimit(50);
	window.setKeyRepeatEnabled(false);

	paddle[1].pos.x = GAME_WF - PADDLE_OFFSET;
	paddle[1].pos.y = GAME_HF / 2.0f;

	paddle[0].pos.x = PADDLE_OFFSET;
	paddle[0].pos.y = GAME_HF / 2.0f;

	reset();

	while (window.isOpen()) {
		sf::Event event;
		while (window.hasFocus() ? window.pollEvent(event) : window.waitEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		update();
		render();
	}
}

void Pog::update() {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
		reset();
	}

	// mouse control
	sf::Vector2i screen = sf::Mouse::getPosition(window);
	sf::Vector2f world = window.mapPixelToCoords(screen);
	paddle[1].pos.y = world.y;

	// ai
	if (serve_timer > 0.0f) {
		paddle[0].pos.y = approach(paddle[0].pos.y, GAME_H / 2.0f, paddle[0].speed);
	} else if (ball.vel.x < 0.0f) {
		float limit = PADDLE_OFFSET + paddle[0].size.x / 2.0f + ball.size.x / 2.0f;
		sf::Vector2f p = ball.pos;
		sf::Vector2f v = ball.vel;

		std::size_t i = 0;
		while (p.x > limit) {
			p += v;
			if (p.y < 0.0f || p.y > GAME_HF) {
				v.y = -v.y;
			}

			if (i++ > 1000) break;
		}

		paddle[0].pos.y = approach(paddle[0].pos.y, p.y, paddle[0].speed);
	}

	// keep in bounds
	for (Paddle& p : paddle) {
		p.pos.y = clamp(p.pos.y, p.size.y / 2.0f, GAME_HF - p.size.y / 2.0f);
	}

	// move
	ball.pos += ball.vel;

	for (Powerup& p : powerups) {
		p.pos += p.vel;
	}

	// collide
	{
		std::size_t i = (ball.vel.x >= 0.0f) ? 1 : 0;
		if (ball.rect().intersects(paddle[i].rect())) {
			sf::Vector2f delta = normalize((ball.pos - ball.vel) - paddle[i].pos);
			float mag = length(ball.vel) + BALL_ACC;

			turn++;

			if (turn >= 10) {
				if (chance(50)) {
					Powerup& p = powerups.emplace_back();
					p.pos.x = GAME_WF / 2.0f;
					p.pos.y = (float)(rand() % GAME_H);
					p.vel.x = 3.0f;
					if (chance(50)) {
						p.type = Powerup::Type::BallSpeedDown;
					} else {
						p.type = Powerup::Type::OpponentSpeedDown;
					}
				}
			}

			if (paddle[i].powerup == Powerup::Type::BallSpeedDown) {
				mag -= 2.0f * BALL_ACC;
			} else if (paddle[i].powerup == Powerup::Type::OpponentSpeedDown) {
				paddle[0].speed -= 0.25f;
			}

			paddle[i].powerup = Powerup::Type::None;

			ball.vel = delta * mag;
		}
	}

	for (auto it = powerups.begin(); it != powerups.end();) {
		if (it->rect().intersects(paddle[1].rect())) {
			paddle[1].powerup = it->type;
			it = powerups.erase(it);
		} else {
			++it;
		}
	}

	// destroy out of bounds
	for (auto it = powerups.begin(); it != powerups.end();) {
		if (it->pos.x < 0.0f || it->pos.x > GAME_WF || it->pos.y < 0.0f || it->pos.y > GAME_HF) {
			it = powerups.erase(it);
		} else {
			++it;
		}
	}

	// bounce
	if (ball.pos.y < 0.0f || ball.pos.y > GAME_HF) {
		ball.vel.y = -ball.vel.y;
	}

	if (ball.pos.x < 0.0f) {
		reset();
		score[1]++;
		serve_dir = -1.0f;
	}

	if (ball.pos.x > GAME_WF) {
		reset();
		score[0]++;
		serve_dir = 1.0f;
	}

	if (serve_timer > 0.0f) {
		serve_timer -= 1.0f;
		if (serve_timer <= 0.0f) {
			ball.vel.x = SERVE_SPEED * serve_dir;
		}
	}
}

void Pog::render() {
	window.clear();
	{
		// objects
		for (Paddle& p : paddle) {
			sf::Color c = sf::Color::White;
			if (p.powerup == Powerup::Type::BallSpeedDown) {
				c = sf::Color::Blue;
			} else if (p.powerup == Powerup::Type::OpponentSpeedDown) {
				c = sf::Color::Cyan;
			}
			draw_rect(window, p.rect(), c);
		}

		draw_rect(window, ball.rect(), sf::Color::White);

		for (Powerup& p : powerups) {
			sf::Color c = sf::Color::White;
			if (p.type == Powerup::Type::BallSpeedDown) {
				c = sf::Color::Blue;
			} else if (p.type == Powerup::Type::OpponentSpeedDown) {
				c = sf::Color::Cyan;
			}
			draw_rect(window, p.rect(), c);
		}

		// line
		for (float y = 0.0f; y < GAME_HF; y += 10.0f) {
			draw_rect(window, sf::FloatRect(GAME_WF / 2.0f - 2.0f, y - 2.0f, 5.0f, 5.0f), sf::Color::White);
		}

		static sf::EasyText ball_spd_label;
		ball_spd_label.setString(
			"BALL SPEED " + std::to_string(length(ball.vel)) +
			"\nTURN " + std::to_string(turn) +
			"\nPADDLE SPEED " + std::to_string(paddle[0].speed)
		);
		ball_spd_label.setFillColor(sf::Color::Red);
		window.draw(ball_spd_label);

		static sf::EasyText ai_score_label;
		ai_score_label.setPosition(GAME_WF / 4.0f, GAME_HF / 10.0f);
		ai_score_label.setScale(2.0f, 2.0f);
		ai_score_label.setString(std::to_string(score[0]));
		window.draw(ai_score_label);

		static sf::EasyText player_score_label;
		player_score_label.setPosition(GAME_WF / 4.0f * 3.0f, GAME_HF / 10.0f);
		player_score_label.setScale(2.0f, 2.0f);
		player_score_label.setString(std::to_string(score[1]));
		window.draw(player_score_label);
	}
	window.display();
}

void Pog::reset() {
	ball.pos.x = GAME_WF / 2.0f;
	ball.pos.y = GAME_HF / 2.0f;

	ball.vel = sf::Vector2f();

	serve_timer = SERVE_TIME;
	turn = 0;
	powerups.clear();
	paddle[1].powerup = Powerup::Type::None;
	paddle[0].speed = PADDLE_SPEED;
}
