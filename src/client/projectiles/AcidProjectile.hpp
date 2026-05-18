#pragma once

#include "Projectile.hpp"

class AcidProjectile : public Projectile {
public:
    AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
	sf::CircleShape m_shape;		
};