#pragma once

#include <SFML/Graphics/Font.hpp>
#include <iostream>

class Globals
{
public:
	static float renderSize;
	static sf::Font FONT_DROID_SANS;

	Globals();
	~Globals();

	static bool InitGlobals()
	{
		renderSize = 50.f;
		std::string fontPath = "../extra_fonts/DroidSans.ttf";
		if (!FONT_DROID_SANS.loadFromFile(fontPath))
		{
			std::cout << "Failed to load " << fontPath << std::endl;
			return false;
		}
		else
		{
			std::cout << "Loaded font " << fontPath << std::endl;
		}

		return true;
	}
};
