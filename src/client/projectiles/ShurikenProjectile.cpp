#include "ShurikenProjectile.hpp"
#include <SFML/Graphics/Texture.hpp>

/**
 * @brief Loads the shuriken texture and scales the sprite to match collision radius.
 * @param ownerId Entity ID of the shooter.
 * @param startPos World position where the shuriken spawns.
 * @param velocity Initial movement vector in pixels per second.
 * @param faction Faction of the owner.
 */
ShurikenProjectile::ShurikenProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Shuriken) {
    
    static sf::Texture shurikenTex;
    static bool texLoaded = false;
    if (!texLoaded) {
        if(shurikenTex.loadFromFile("assets/textures/shuriken.png")) {
            shurikenTex.setSmooth(false); 
        }
        texLoaded = true;
    }
    
    m_sprite.emplace(shurikenTex);
    
    float texWidth = m_sprite->getLocalBounds().size.x;
    float texHeight = m_sprite->getLocalBounds().size.y;
    m_sprite->setOrigin({texWidth / 2.0f, texHeight / 2.0f});
    
    if (texWidth > 0.0f) {
        float scale = (m_radius * 2.0f) / texWidth;
        m_sprite->setScale({scale, scale});
    }
}

/**
 * @brief Advances base movement and rotates the sprite each frame.
 * @param deltaTime Elapsed time since the last update.
 */
void ShurikenProjectile::update(sf::Time deltaTime) {
    Projectile::update(deltaTime);
    
    m_sprite->rotate(sf::degrees(1200.0f * deltaTime.asSeconds()));
}

/**
 * @brief Draws the shuriken sprite at its current position.
 * @param target Render target to draw into.
 */
void ShurikenProjectile::render(sf::RenderTarget& target) {
    m_sprite->setPosition(m_position);
    target.draw(*m_sprite);
}
