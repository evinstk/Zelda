#include "composite_collider.h"

#include <algorithm>

namespace te
{
	void CompositeCollider::addCollider(const BoxCollider& collider)
	{
		mBoxColliders.push_back(collider);
		std::vector<Wall2f> walls = collider.getWalls();
		mWalls.insert(mWalls.end(), walls.begin(), walls.end());
	}

	const std::vector<Wall2f>& CompositeCollider::getWalls() const
	{
		return mWalls;
	}

	//std::vector<Wall2f> CompositeCollider::getWalls() const
	//{
	//	std::vector<Wall2f> walls;
	//	std::for_each(mBoxColliders.begin(), mBoxColliders.end(), [&walls](const BoxCollider& collider) {
	//		std::vector<Wall2f> currWalls = collider.getWalls();
	//		walls.insert(walls.end(), currWalls.begin(), currWalls.end());
	//	});
	//	return walls;
	//}

	bool CompositeCollider::contains(float x, float y) const
	{
		for (auto it = mBoxColliders.begin(); it != mBoxColliders.end(); ++it)
		{
			if (it->contains(x, y)) return true;
		}
		return false;
	}

	bool CompositeCollider::intersects(const BoxCollider& o) const
	{
		for (auto& boxCollider : mBoxColliders)
			if (boxCollider.intersects(o)) return true;
		return false;
	}

	bool CompositeCollider::intersects(const BoxCollider& o, sf::FloatRect& collision) const
	{
		bool result = false;

		sf::FloatRect currBest = { 0, 0, 0, 0 };
		sf::FloatRect currCollision;
		for (auto& boxCollider : mBoxColliders)
		{
			if (boxCollider.intersects(o, currCollision))
			{
				if (currCollision.width * currCollision.height > currBest.width * currBest.height)
				{
					currBest = currCollision;
				}
				result = true;
			}
		}

		collision = currBest;
		return result;
	}

	bool CompositeCollider::intersects(const CompositeCollider& o) const
	{
		for (auto& boxCollider : mBoxColliders)
			if (o.intersects(boxCollider)) return true;
		return false;
	}

	bool CompositeCollider::intersects(const CompositeCollider& o, sf::FloatRect& collision) const
	{
		bool result = false;

		sf::FloatRect currBest = { 0, 0, 0, 0 };
		sf::FloatRect currCollision;
		for (auto& boxCollider : mBoxColliders)
		{
			if (o.intersects(boxCollider, currCollision))
			{
				if (currCollision.width * currCollision.height > currBest.width * currBest.height)
				{
					currBest = currCollision;
				}
			}
		}

		collision = currBest;
		return result;
	}

	CompositeCollider CompositeCollider::transform(const sf::Transform& t) const
	{
		CompositeCollider newComposite;
		newComposite.mBoxColliders.reserve(mBoxColliders.size());
		for (auto& collider : mBoxColliders)
			newComposite.addCollider(collider.transform(t));
		return newComposite;
	}

	void CompositeCollider::createFixtures(b2Body& body, std::vector<b2Fixture*>& outFixtures) const
	{
		outFixtures.clear();
		outFixtures.reserve(mBoxColliders.size());
		for (auto& boxCollider : mBoxColliders)
		{
			outFixtures.push_back(boxCollider.createFixture(body));
		}
	}

	void CompositeCollider::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		std::for_each(mBoxColliders.begin(), mBoxColliders.end(), [&target, &states](const BoxCollider& collider) {
			target.draw(collider, states);
		});
	}
}
