#include "EasyText.h"
#include "stb_easy_font.h"

namespace sf
{
	std::vector<EasyText::Vertex> EasyText::m_tmp;

	void sf::EasyText::setString(const std::string& string) {
		if (m_string != string) {
			m_string = string;
			m_outdated = true;
		}
	}

	void EasyText::setFillColor(const Color& fillColor) {
		if (m_fillColor != fillColor) {
			m_fillColor = fillColor;
			m_outdated = true;
		}
	}

	void EasyText::setLetterSpacing(float spacing) {
		if (m_spacing != spacing) {
			m_spacing = spacing;
			m_outdated = true;
		}
	}

	void EasyText::draw(RenderTarget& target, RenderStates states) const {
		if (m_outdated) {
			if (m_tmp.empty()) {
				m_tmp.resize(1000);
			}

			stb_easy_font_spacing(m_spacing);

			size_t num_quads = (size_t)stb_easy_font_print(
				0.0f,
				0.0f,
				m_string.c_str(),
				nullptr,
				m_tmp.data(),
				(int)(m_tmp.size() * sizeof(Vertex))
			);

			m_buffer.resize(num_quads * 4);

			for (size_t i = 0; i < num_quads * 4; i++) {
				m_buffer[i].position.x = m_tmp[i].x;
				m_buffer[i].position.y = m_tmp[i].y;
				m_buffer[i].color = m_fillColor;
			}

			m_outdated = false;
		}

		states.transform *= getTransform();
		target.draw(m_buffer, states);
	}
}
