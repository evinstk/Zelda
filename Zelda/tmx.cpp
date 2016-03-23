#include "tmx.h"

#include "texture_manager.h"
#include "tile_map.h"
#include "composite_collider.h"

#include <SFML/Graphics.hpp>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>

#include <memory>
#include <algorithm>
#include <array>

namespace te
{
	const int TMX::NULL_TILE = -1;
	const TMX::TileData TMX::NULL_DATA = TMX::TileData{ NULL_TILE, TMX::ObjectGroup() };

	TMX::TMX(const std::string& filename)
	{
		rapidxml::file<> tmxFile(filename.c_str());
		rapidxml::xml_document<> tmx;
		tmx.parse<0>(tmxFile.data());

		rapidxml::xml_node<char>* pMapNode = tmx.first_node("map");
		mWidth = std::stoi(pMapNode->first_attribute("width")->value());
		mHeight = std::stoi(pMapNode->first_attribute("height")->value());
		mTilewidth = std::stoi(pMapNode->first_attribute("tilewidth")->value());
		mTileheight = std::stoi(pMapNode->first_attribute("tileheight")->value());

		for (rapidxml::xml_node<char>* pTileset = tmx.first_node("map")->first_node("tileset"); pTileset != 0; pTileset = pTileset->next_sibling("tileset"))
		{
			std::vector<TileData> tiles;
			for (rapidxml::xml_node<char>* pTile = pTileset->first_node("tile"); pTile != 0; pTile = pTile->next_sibling("tile"))
			{
				rapidxml::xml_node<char>* pObjectGroup = pTile->first_node("objectgroup");
				std::vector<Object> objects;
				for (rapidxml::xml_node<char>* pObject = pObjectGroup->first_node("object"); pObject != 0; pObject = pObject->next_sibling("object"))
				{
					objects.push_back(Object{
						std::stoi(pObject->first_attribute("id")->value()),
						std::stoi(pObject->first_attribute("x")->value()),
						std::stoi(pObject->first_attribute("y")->value()),
						pObject->first_attribute("width") != 0 ? std::stoi(pObject->first_attribute("width")->value()) : 0,
						pObject->first_attribute("height") != 0 ? std::stoi(pObject->first_attribute("height")->value()) : 0,
					});
				}

				tiles.push_back({
					std::stoi(pTile->first_attribute("id")->value()),
					ObjectGroup {
						pObjectGroup->first_attribute("draworder")->value(),
						std::move(objects)
					}
				});
			}

			rapidxml::xml_node<char>* pImage = pTileset->first_node("image");

			mTilesets.push_back({
				std::stoi(pTileset->first_attribute("firstgid")->value()),
				pTileset->first_attribute("name")->value(),
				std::stoi(pTileset->first_attribute("tilewidth")->value()),
				std::stoi(pTileset->first_attribute("tileheight")->value()),
				std::stoi(pTileset->first_attribute("tilecount")->value()), {
					pImage->first_attribute("source")->value(),
					std::stoi(pImage->first_attribute("width")->value()),
				    std::stoi(pImage->first_attribute("height")->value())
				},
				std::move(tiles)
			});
		}

		for (rapidxml::xml_node<char>* pLayer = tmx.first_node("map")->first_node("layer"); pLayer != 0; pLayer = pLayer->next_sibling("layer"))
		{
			std::vector<Tile> tiles;
			for (rapidxml::xml_node<char>* pTile = pLayer->first_node("data")->first_node("tile"); pTile != 0; pTile = pTile->next_sibling("tile"))
			{
				tiles.push_back({
					std::stoi(pTile->first_attribute("gid")->value())
				});
			}
			mLayers.push_back({
				pLayer->first_attribute("name")->value(),
				std::stoi(pLayer->first_attribute("width")->value()),
				std::stoi(pLayer->first_attribute("height")->value()),
				{std::move(tiles)}
			});
		}
	}

	static std::vector<TMX::Tileset>::const_iterator getTilesetIterator(int gid, const std::vector<TMX::Tileset>& tilesets)
	{
		auto retIt = tilesets.end();
		for (auto it = tilesets.begin(); it != tilesets.end(); ++it)
		{
			if (gid >= it->firstgid && gid < it->firstgid + it->tilecount)
			{
				retIt = it;
			}
		}
		if (retIt == tilesets.end())
		{
			throw std::out_of_range("GID does not exist for TMX.");
		}
		return retIt;
	}

	TileMap TMX::makeTileMap(TextureManager& textureManager) const
	{
		std::vector<std::shared_ptr<sf::Texture>> pTextures;
		std::for_each(mTilesets.begin(), mTilesets.end(), [&pTextures, &textureManager](const Tileset& tileset) {
			pTextures.push_back(textureManager.get(tileset.image.source));
		});

		std::vector<std::vector<sf::VertexArray>> layers;
		std::for_each(mLayers.begin(), mLayers.end(), [this, &layers](const Layer& layer) {
			std::vector<sf::VertexArray> vertexArrays(mTilesets.size());
			std::for_each(vertexArrays.begin(), vertexArrays.end(), [](sf::VertexArray& va) {
				va.setPrimitiveType(sf::Quads);
			});

			for (auto tileIter = layer.data.tiles.begin(); tileIter != layer.data.tiles.end(); ++tileIter) {
				if (tileIter->gid != 0)
				{
					int tileIndex = tileIter - layer.data.tiles.begin();
					int x = tileIndex % mWidth;
					int y = tileIndex / mWidth;

					std::array<sf::Vertex, 4> quad;
					quad[0].position = sf::Vector2f((float)x * mTilewidth, (float)y * mTileheight);
					quad[1].position = sf::Vector2f((x + 1.f) * mTilewidth, (float)y * mTileheight);
					quad[2].position = sf::Vector2f((x + 1.f) * mTilewidth, (y + 1.f) * mTileheight);
					quad[3].position = sf::Vector2f((float)x * mTilewidth, (y + 1.f) * mTileheight);

					auto tilesetIter = getTilesetIterator(tileIter->gid, mTilesets);
					int localId = tileIter->gid - tilesetIter->firstgid;
					int tu = localId % (tilesetIter->image.width / tilesetIter->tilewidth);
					int tv = localId / (tilesetIter->image.width / tilesetIter->tilewidth);

					quad[0].texCoords = sf::Vector2f((float)tu * mTilewidth, (float)tv * mTileheight);
					quad[1].texCoords = sf::Vector2f((tu + 1.f) * mTilewidth, (float)tv * mTileheight);
					quad[2].texCoords = sf::Vector2f((tu + 1.f) * mTilewidth, (tv + 1.f) * mTileheight);
					quad[3].texCoords = sf::Vector2f((float)tu * mTilewidth, (tv + 1.f) * mTileheight);

					int tilesetIndex = tilesetIter - mTilesets.begin();
					std::for_each(quad.begin(), quad.end(), [&vertexArrays, tilesetIndex](sf::Vertex& v) {
						vertexArrays[tilesetIndex].append(v);
					});
				}
			}

			layers.push_back(vertexArrays);
		});

		return TileMap(pTextures, layers);
	}

	const TMX::TileData& TMX::getTileData(int x, int y, const TMX::Layer& layer) const
	{
		int i = y * mWidth + x;
		if (i >= mWidth * mHeight)
		{
			throw std::out_of_range("Tile coordinates are out of bounds.");
		}
		const Tile& tile = layer.data.tiles[i];
		if (tile.gid == 0)
		{
			return TMX::NULL_DATA;
		}
		auto tilesetIter = getTilesetIterator(tile.gid, mTilesets);
		const int localId = tile.gid - tilesetIter->firstgid;
		const std::vector<TileData>& tiles = tilesetIter->tiles;
		auto result = std::find_if(tiles.begin(), tiles.end(), [localId](const TileData& data) {
			return localId == data.id;
		});
		if (result == tiles.end())
		{
			return TMX::NULL_DATA;
		}
		return *result;
	}

	CompositeCollider TMX::makeCollider() const
	{
		CompositeCollider collider;
		std::for_each(mLayers.begin(), mLayers.end(), [&collider, this](const Layer& layer) {
			for (int y = 0; y < mHeight; ++y)
			{
				for (int x = 0; x < mWidth; ++x)
				{
					const TMX::TileData& tileData = getTileData(x, y, layer);
					if (tileData.id != NULL_TILE)
					{
						sf::Transform transform;
						transform.translate((float)x * mTilewidth, (float)y * mTileheight);
						std::for_each(tileData.objectgroup.objects.begin(), tileData.objectgroup.objects.end(), [&collider, &transform](const Object& obj) {
							collider.addCollider({ transform.transformRect({ (float)obj.x, (float)obj.y, (float)obj.width, (float)obj.height }) });
						});
					}
				}
			}
		});
		return collider;
	}
}
