#pragma once

#include "Entity.hpp"
#include "Config.hpp"

class Player : public Entity{
public:
    Player(std::uint32_t id, const sf::Vector2f& startPos);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void setFocused(bool focuesd);

private:
    sf::CircleShape m_shape;
    float m_speed;
    bool m_isFocused;
};