#pragma once

#include <SFML/Graphics.hpp>

class Entity{
public:
    Entity(std::uint32_t id, const sf::Vector2f& startPos): m_id(id), m_position(startPos) {}

    virtual ~Entity() = default;

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    std::uint32_t getId() const {return m_id;}
    sf::Vector2f getPosition() const {return m_position;}
    void setPosition(const sf::Vector2f&  pos) {m_position = pos;} 

protected:
    std::uint32_t m_id;
    sf::Vector2f m_position;
};