#include "tile_map.h"
#include "texture_manager.h"
#include "vector_ops.h"
#include "game.h"

#include <algorithm>
#include <limits>

namespace te
{
	static float calculateAverageGraphEdgeLength(const TileMap::NavGraph& navGraph)
	{
		float totalLength = 0;
		int numEdgesCounted = 0;

		TileMap::NavGraph::ConstNodeIterator nodeIter(navGraph);
		for (const TileMap::NavGraph::Node* pNode = nodeIter.begin(); !nodeIter.end(); pNode = nodeIter.next())
		{
			TileMap::NavGraph::ConstEdgeIterator edgeIter(navGraph, pNode->getIndex());
			for (const TileMap::NavGraph::Edge* pEdge = edgeIter.begin(); !edgeIter.end(); pEdge = edgeIter.next())
			{
				++numEdgesCounted;
				totalLength += distance(navGraph.getNode(pEdge->getFrom()).getPosition(), navGraph.getNode(pEdge->getTo()).getPosition());
			}
		}

		return totalLength / numEdgesCounted;
	}

	TileMap::TileMap(Game& world, TextureManager& textureManager, const TMX& tmx)
		: BaseGameEntity(world, b2BodyDef())
		, mWorld(world)
		, mTextures()
		, mpCollider(nullptr)
		, mpNavGraph(nullptr)
		, mDrawFlags(0)
		, mCellSpaceNeighborhoodRange(1)
		, mpCellSpacePartition(nullptr)
	{
		setDrawOrder(std::numeric_limits<int>::max());

		std::vector<std::vector<sf::VertexArray>> layers;
		tmx.makeVertices(textureManager, mTextures, layers);
		for (auto it = layers.begin(); it != layers.end(); ++it)
		{
			int index = it - layers.begin();
			if (it->size() != mTextures.size()) {
				throw std::runtime_error("Texture and layer component counts are inconsistent.");
			}
			auto pLayer = std::make_unique<Layer>(mWorld, std::move(*it), mTextures);
			pLayer->setDrawOrder(index);
			attachNode(std::move(pLayer));
		}

		const sf::Transform& transform = getWorld().getPixelToWorldTransform();

		mpCollider = std::unique_ptr<CompositeCollider>(tmx.makeCollider(transform));

		mpNavGraph = std::unique_ptr<NavGraph>(tmx.makeNavGraph(transform));

		mCellSpaceNeighborhoodRange = calculateAverageGraphEdgeLength(*mpNavGraph) + 1;

		mpCellSpacePartition = std::make_unique<NavCellSpace>((float)tmx.getTileWidth() / tmx.getWidth(), (float)tmx.getTileHeight() * tmx.getHeight(), tmx.getWidth() / 4, tmx.getHeight() / 4, mpNavGraph->numNodes());

		TileMap::NavGraph::ConstNodeIterator nodeIter(*mpNavGraph);
		for (const TileMap::NavGraph::Node* pNode = nodeIter.begin(); !nodeIter.end(); pNode = nodeIter.next())
		{
			mpCellSpacePartition->addEntity(pNode);
		}

		std::vector<b2Fixture*> fixtures;
		mpCollider->createFixtures(getBody(), fixtures);
	}

	const std::vector<Wall2f>& TileMap::getWalls() const
	{
		return mpCollider->getWalls();
	}

	const TileMap::NavGraph& TileMap::getNavGraph() const
	{
		return *mpNavGraph;
	}

	void TileMap::setDrawColliderEnabled(bool enabled)
	{
		mDrawFlags = enabled ? mDrawFlags | COLLIDER : mDrawFlags ^ COLLIDER;
	}

	void TileMap::setDrawNavGraphEnabled(bool enabled)
	{
		mDrawFlags = enabled ? mDrawFlags | NAV_GRAPH : mDrawFlags ^ NAV_GRAPH;
		if (enabled)
			mpNavGraph->prepareVerticesForDrawing();
	}

	float TileMap::getCellSpaceNeighborhoodRange() const
	{
		return mCellSpaceNeighborhoodRange;
	}

	TileMap::NavCellSpace& TileMap::getCellSpace()
	{
		return *mpCellSpacePartition;
	}

	bool TileMap::intersects(const BoxCollider& o) const
	{
		return mpCollider->transform(getWorldTransform()).intersects(o);
	}

	bool TileMap::intersects(const BoxCollider& o, sf::FloatRect& collision) const
	{
		return mpCollider->transform(getWorldTransform()).intersects(o, collision);
	}

	bool TileMap::intersects(const CompositeCollider& o) const
	{
		return mpCollider->transform(getWorldTransform()).intersects(o);
	}

	bool TileMap::intersects(const CompositeCollider& o, sf::FloatRect& collision) const
	{
		return mpCollider->transform(getWorldTransform()).intersects(o, collision);
	}

	void TileMap::onDraw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getWorldTransform();

		states.texture = NULL;
		if ((mDrawFlags & COLLIDER) > 0)
		{
			target.draw(*mpCollider, states);
		}

		if ((mDrawFlags & NAV_GRAPH) > 0)
		{
			target.draw(*mpNavGraph, states);
		}
	}

	TileMap::Layer::Layer(Game& world, std::vector<sf::VertexArray>&& vas, std::vector<const sf::Texture*>& textures)
		: BaseGameEntity(world, b2BodyDef())
		, mVertexArrays(std::move(vas))
		, mTextures(&textures)
	{}

	void TileMap::Layer::onDraw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getWorldTransform() * getWorld().getPixelToWorldTransform();

		for (auto iter = mVertexArrays.begin(); iter != mVertexArrays.end(); ++iter)
		{
			states.texture = (*mTextures)[iter - mVertexArrays.begin()];
			target.draw(*iter, states);
		}
	}
}
