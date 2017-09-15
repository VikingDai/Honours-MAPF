#include "Input.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>
#include "Simulation.h"

#include <SFML/Window.hpp>
#include "Globals.h"

const int KEY_SPACE_BAR = 32;

sf::Vector2f Input::GetMouseWorld()
{
	sf::Vector2i& mousePosi = sf::Mouse::getPosition(*window);
	sf::Vector2f mousePos(mousePosi.x, mousePosi.y);

	sf::Vector2f windowSize(window->getSize().x, window->getSize().y);

	sf::Vector2f& offset = view->getSize() * 0.5f - view->getCenter();
	mousePos -= offset * (1 / zoom);

	mousePos.x = round(mousePos.x / (Globals::renderSize * (1 / zoom)));
	mousePos.y = round(mousePos.y / (Globals::renderSize * (1 / zoom)));

	return mousePos;
}

Tile* Input::GetTileUnderCursor()
{
	sf::Vector2f& mousePos = GetMouseWorld();
	Tile* tile = simulation->environment.gridMap.GetTileAt(mousePos.x, mousePos.y);
	return tile;
}

void Input::ProcessInput(sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::MouseMoved:
	{
		Tile* newHoveredTile = GetTileUnderCursor();

		if (hoveredTile != newHoveredTile)
		{
			if (hoveredTile && hoveredTile != startTile && hoveredTile != goalTile)
				hoveredTile->ResetColor();

			if (newHoveredTile)
			{
				std::cout << *newHoveredTile << std::endl;
				if (newHoveredTile != startTile && newHoveredTile != goalTile)
				{
					newHoveredTile->SetColor(sf::Color::Magenta);
				}
			}
		}

		hoveredTile = newHoveredTile;

		break;
	}
	case sf::Event::MouseWheelMoved:
	{
		float zoomScale = 1.f + event.mouseWheel.delta * -0.1f;
		view->zoom(zoomScale);
		zoom *= zoomScale;
		break;
	}
	case sf::Event::KeyPressed:
	{
		OnKeyPressed(event);
		break;
	}
	case sf::Event::MouseButtonPressed:
	{
		OnMousePressed(event);
		break;
	}
	case sf::Event::MouseButtonReleased:
	{
		OnMouseReleased(event);
		break;
	}
	}
}

void Input::OnKeyPressed(sf::Event& event)
{
	// event.key.code
}

void Input::OnMousePressed(sf::Event& event)
{
	//event.mouseButton.button

	switch (event.mouseButton.button)
	{
	case sf::Mouse::Left:
	{
		Tile* tile = GetTileUnderCursor();
		if (!tile) return;

		if (startTile != nullptr)
			startTile->ResetColor();

		startTile = tile;
		startTile->SetColor(sf::Color::Blue);
		break;
	}
	case sf::Mouse::Right:
	{
		Tile* tile = GetTileUnderCursor();
		if (!tile) return;

		if (goalTile)
			goalTile->ResetColor();

		goalTile = tile;
		goalTile->SetColor(sf::Color::Green);

		if (startTile)
			simulation->temporalAStar.FindPath(startTile, goalTile);

		break;
	}
	}
}

void Input::OnMouseReleased(sf::Event& event)
{
	//event.mouseButton.button
}

void Input::Update(float deltaTime, Simulation* simulation)
{
	if (ImGui::IsKeyPressed(KEY_SPACE_BAR))
	{
		StepSimulation(simulation);
	}

	//if (ImGui::IsMouseClicked(0))
	//{
	//	camera->getMVP();
	//	camera->zoom;
	//	camera->position;
	//	
	//	ImVec2 mouseWorld = ImVec2(
	//		ImGui::GetMousePos().x - Graphics::GetDisplaySize().x * 0.5f,
	//		ImGui::GetMousePos().y - Graphics::GetDisplaySize().y * 0.5f);

	//	std::cout << ImGui::GetMousePos().x << " / " << Graphics::GetDisplaySize().x << std::endl;

	//	std::cout << mouseWorld.x << " , " << mouseWorld.y << std::endl;

	//	simulation->SelectTile(
	//		(ImGui::GetMousePos().x - Graphics::GetDisplaySize().x * 0.5f + camera->position.x) / camera->zoom,
	//		(ImGui::GetMousePos().y - Graphics::GetDisplaySize().y * 0.5f + camera->position.y) / camera->zoom);
	//}

	//// Update camera
	//if (ImGui::IsMouseDown(0))
	//{
	//	camera->position.x += -1 * ImGui::GetIO().MouseDelta.x * deltaTime * camera->zoom / 100.f;
	//	camera->position.y += ImGui::GetIO().MouseDelta.y * deltaTime * camera->zoom / 100.f;
	//}

	//camera->zoom *= 1 - (0.075f * ImGui::GetIO().MouseWheel);
}

void Input::StepSimulation(Simulation* simulation)
{
	simulation->Step();
}

void Input::Reset()
{
	startTile = goalTile = hoveredTile = nullptr;
}
