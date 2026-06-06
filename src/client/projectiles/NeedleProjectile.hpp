#pragma once

#include "Projectile.hpp"

class NeedleProjectile : public Projectile {
public:
    NeedleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void render(sf::RenderTarget& target) override;
    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;

private:
    sf::ConvexShape m_needleShape;
};
