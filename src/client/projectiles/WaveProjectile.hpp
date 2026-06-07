#pragma once
#include "Projectile.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <unordered_set>

class WaveProjectile : public Projectile {
public:
    WaveProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;
    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;

private:
    std::unordered_set<std::uint32_t> m_hitEntities;

    sf::VertexArray m_mesh;
    float m_facingAngle;
};
