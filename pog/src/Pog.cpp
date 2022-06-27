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

static void draw_rect(sf::RenderTarget& target, sf::Vector2f pos, sf::Vector2f size) {
	sf::RectangleShape r;
	r.setPosition(pos);
	r.setSize(size);
	r.setOrigin(size / 2.0f);
	target.draw(r);
}

void Pog::run() {
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
	// mouse control
	sf::Vector2i screen = sf::Mouse::getPosition(window);
	sf::Vector2f world = window.mapPixelToCoords(screen);
	paddle[1].pos.y = world.y;

	// ai
	if (serve_timer > 0.0f) {
		paddle[0].pos.y = approach(paddle[0].pos.y, GAME_H / 2.0f, PADDLE_SPEED);
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

		paddle[0].pos.y = approach(paddle[0].pos.y, p.y, PADDLE_SPEED);
	}

	// keep in bounds
	for (Paddle& p : paddle) {
		p.pos.y = clamp(p.pos.y, p.size.y / 2.0f, GAME_HF - p.size.y / 2.0f);
	}

	ball.pos += ball.vel;

	// collide
	{
		std::size_t i = (ball.vel.x >= 0.0f) ? 1 : 0;
		if (ball.rect().intersects(paddle[i].rect())) {
			sf::Vector2f delta = normalize((ball.pos - ball.vel) - paddle[i].pos);
			float mag = length(ball.vel);
			ball.vel = delta * (mag + BALL_ACC);
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
		for (Paddle& p : paddle) draw_rect(window, p.pos, p.size);

		draw_rect(window, ball.pos, ball.size);

		for (float y = 0.0f; y < GAME_HF; y += 10.0f) {
			draw_rect(window, sf::Vector2f(GAME_WF / 2.0f, y), sf::Vector2f(5.0f, 5.0f));
		}

		static sf::EasyText ai_score;
		ai_score.setPosition(GAME_WF / 4.0f, GAME_HF / 10.0f);
		ai_score.setScale(2.0f, 2.0f);
		ai_score.setString(std::to_string(score[0]));
		window.draw(ai_score);

		static sf::EasyText player_score;
		player_score.setPosition(GAME_WF / 4.0f * 3.0f, GAME_HF / 10.0f);
		player_score.setScale(2.0f, 2.0f);
		player_score.setString(std::to_string(score[1]));
		window.draw(player_score);

		static sf::EasyText ball_spd;
		ball_spd.setString("BALL SPEED " + std::to_string(length(ball.vel)));
		ball_spd.setFillColor(sf::Color::Red);
		window.draw(ball_spd);
	}
	window.display();
}

void Pog::reset() {
	ball.pos.x = GAME_WF / 2.0f;
	ball.pos.y = GAME_HF / 2.0f;

	ball.vel = sf::Vector2f();

	serve_timer = SERVE_TIME;
}
