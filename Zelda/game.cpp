#include "game.h"
#include "tile_map.h"
#include "vector_ops.h"

namespace te
{
	Game::~Game() {}

	bool Game::isPathObstructed(sf::Vector2f a, sf::Vector2f b, float boundingRadius) const
	{
		throwIfNoMap();
		auto& walls = mpTileMap->getWalls();

		sf::Vector2f toB = normalize(b - a);
		sf::Vector2f currPos = a;

		while (distanceSq(currPos, b) > boundingRadius * boundingRadius)
		{
			currPos += toB * 0.5f * boundingRadius;
			for (auto& wall : walls)
				if (wall.intersects(currPos, boundingRadius))
					return true;
		}

		return false;
	}

	void Game::setTileMap(const std::shared_ptr<TileMap>& pTileMap)
	{
		if (pTileMap)
		{
			mpTileMap = pTileMap;
		}
		else
		{
			throw std::runtime_error("Must supply TileMap to Game.");
		}
	}

	void Game::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		throwIfNoMap();
		states.transform *= getTransform();
		target.draw(*mpTileMap, states);
	}

	void Game::throwIfNoMap() const
	{
		if (!mpTileMap) throw std::runtime_error("TileMap not set in Game.");
	}
}