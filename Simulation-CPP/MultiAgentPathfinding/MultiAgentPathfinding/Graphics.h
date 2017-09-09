#pragma once

#include <SFML/Graphics.hpp>

using namespace sf;

namespace MyGraphics
{
	static RectangleShape CreateRect(sf::Vector2f size, sf::Vector2f pos)
	{
		sf::Vector2f alteredSize = size * 50.f;
		sf::Vector2f alteredPos = pos * 50.f;
		RectangleShape rect(alteredSize);
		rect.setOrigin(alteredSize * 0.5f);
		rect.setPosition(alteredPos);
		return rect;
	}

	static CircleShape CreateCirc(float radius, sf::Vector2f pos)
	{
		float alteredRadius = radius * 50.f;
		sf::Vector2f alteredPos = pos * 50.f;
		CircleShape circ(alteredRadius);
		circ.setOrigin(sf::Vector2f(alteredRadius, alteredRadius));
		circ.setPosition(alteredPos);
		return circ;
	}
}