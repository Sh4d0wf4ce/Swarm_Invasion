#pragma once

#include "Entity.hpp"
#include "EnemyRegistry.hpp"

class Enemy : public Entity{
public:
    Enemy(std::uint32_t id, const sf::Vector2f& startPos, EnemyType type);

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;

    EnemyType getType() const { return m_type; }
    Faction getFaction()  const override { return Faction::Enemies; }
    float getRadius() const override { return EnemyRegistry::getStats(m_type).radius; }

private:
    sf::CircleShape m_shape;
    EnemyType m_type;
};