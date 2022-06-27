#pragma once

#include <SFML/Graphics.hpp>

namespace sf
{
	class EasyText : public Drawable, public Transformable
	{
	public:
		void setString(const std::string& string);
		void setFillColor(const Color& fillColor);
		void setLetterSpacing(float spacing);

		inline const std::string& getString() const { return m_string; }
		inline const Color& getFillColor() const { return m_fillColor; }
		inline float getLetterSpacing() const { return m_spacing; }

	private:
		struct Vertex
		{
			float x, y, z;
			uint8_t r, g, b, a;
		};

		void draw(RenderTarget& target, RenderStates states) const override;

		static std::vector<Vertex> m_tmp;

		std::string m_string;
		Color m_fillColor = Color::White;
		float m_spacing = 0.0f;
		mutable VertexArray m_buffer{sf::Quads};
		mutable bool m_outdated = false;
	};
}
