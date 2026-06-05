#pragma once
#include "Projectile.hpp"
#include <optional>
#include <SFML/Graphics/Sprite.hpp>

class ShurikenProjectile : public Projectile {
public:
    ShurikenProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    std::optional<sf::Sprite> m_sprite;
};