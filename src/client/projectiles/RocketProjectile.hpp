#pragma once

#include "Projectile.hpp"

class RocketProjectile : public Projectile {
public:
    RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;

private:
    sf::CircleShape m_shape;
    float m_explosionRadius{80.0f};
};