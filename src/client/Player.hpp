#pragma once

#include "Entity.hpp"

class Player : public Entity{
public:
    Player(std::uint32_t id, const sf::Vector2f& startPos);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    sf::CircleShape m_shape;
    float m_speed;
};