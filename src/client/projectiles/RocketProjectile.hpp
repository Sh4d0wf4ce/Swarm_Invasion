#pragma once
#include "Projectile.hpp"
#include <optional>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/CircleShape.hpp>

/**
 * @brief Explosive rocket projectile with area-of-effect detonation and fade-out visuals.
 *
 * On impact with a wall or entity, triggers an explosion that damages all enemies
 * within the configured radius before deactivating.
 */
class RocketProjectile : public Projectile {
public:
    RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;
    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;
private:
    std::optional<sf::Sprite> m_sprite;
    sf::CircleShape m_explosionShape;

    
    // Explosion State
    bool m_isExploding{false};
    float m_explosionTimer{0.0f};
    sf::Vector2f m_explosionPos;
    float m_explosionRadius{80.0f};
    const float m_maxExplosionTime{0.4f};
};
