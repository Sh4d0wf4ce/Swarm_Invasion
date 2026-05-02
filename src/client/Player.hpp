#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"

class Player : public Entity{
public:
    Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass);

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;

    void setFocused(bool focuesd);

    PlayerClass getClass() const { return m_class; }

private:
    bool checkCollision(const sf::Vector2f& pos, const std::shared_ptr<MapGenerator>& map);

    sf::CircleShape m_shape;
    float m_speed;
    bool m_isFocused;
    PlayerClass m_class;
};