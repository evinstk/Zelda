#ifndef TE_SCENE_NODE_H
#define TE_SCENE_NODE_H

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>
#include <functional>

struct b2BodyDef;
class b2Body;

namespace te
{
	class Game;

	class SceneNode : public sf::Drawable
	{
	public:
		static std::unique_ptr<SceneNode> make(Game& world, const b2BodyDef&);

		virtual ~SceneNode();

		void setPosition(sf::Vector2f position);
		sf::Vector2f getPosition() const;

		void setDrawOrder(int z);
		int getDrawOrder() const;

		sf::Transform getWorldTransform() const;

		void attachNode(std::unique_ptr<SceneNode>&& child);
		std::unique_ptr<SceneNode> detachNode(const SceneNode& child);
	private:
		struct PendingDraw
		{
			sf::Transform transform;
			const SceneNode* pNode;
		};

		SceneNode(Game& world, const b2BodyDef&);

		sf::Transform getParentTransform() const;
		void draw(sf::RenderTarget&, sf::RenderStates) const;
		virtual void onDraw(sf::RenderTarget&, sf::RenderStates) const;
		void concatPendingDraws(std::vector<PendingDraw>& outQueue) const;

		Game& mWorld;
		SceneNode* mpParent;
		std::unique_ptr<b2Body, std::function<void(b2Body*)>> mBody;
		std::vector<std::unique_ptr<SceneNode>> mChildren;
		int mZ;
	};
}

#endif
