#pragma once

#include "Projectile.hpp"

class RocketProjectile : public Projectile {
public:
    RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Rocket) {
        
        m_lifetime = 5.0f;
        m_shape.setRadius(m_radius);
        m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
        m_shape.setOrigin({m_radius, m_radius});
    }

    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;

private:
    float m_explosionRadius{80.0f};
};