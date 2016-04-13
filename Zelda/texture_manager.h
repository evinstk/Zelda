#ifndef TE_TEXTURE_MANAGER_H
#define TE_TEXTURE_MANAGER_H

#include "typedefs.h"
#include "texture_atlas.h"
#include "animation.h"

#include <memory>
#include <map>

namespace sf
{
	class Texture;
	class Sprite;
}

namespace te
{
	class TextureManager
	{
	public:
		static std::unique_ptr<TextureManager> make();

		static TextureID getID(const std::string& filename);

		TextureID load(const std::string& filename);
		TextureID loadSpritesheet(const std::string& dir, const std::string& xmlFile);
		TextureID loadAnimations(const std::string& filename);

		const sf::Texture& get(TextureID file) const;
		const sf::Sprite& getSprite(TextureID sprite) const;
		const Animation& getAnimation(TextureID animation) const;
	private:
		TextureManager();

		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;

		std::map<TextureID, std::unique_ptr<sf::Texture>> mTextures;
		std::map<TextureID, std::unique_ptr<sf::Sprite>> mSpriteMap;
		std::map<TextureID, std::unique_ptr<Animation>> mAnimationMap;
	};
}

#endif
