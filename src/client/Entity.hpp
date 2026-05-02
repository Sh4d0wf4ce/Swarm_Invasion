#pragma once

#include "MapGenerator.hpp"
#include "Config.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

class Entity{
public:
    Entity(std::uint32_t id, const sf::Vector2f& startPos): m_id(id), m_position(startPos) {}

    virtual ~Entity() = default;

    virtual void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    std::uint32_t getId() const {return m_id;}
    sf::Vector2f getPosition() const {return m_position;}

    void setPosition(const sf::Vector2f&  pos) {m_position = pos;} 
    void setHp(float current){
        m_hp = current;
    }

protected:
    std::uint32_t m_id;
    sf::Vector2f m_position;

    float m_hp{100.0f};
    float m_maxHp{100.0f};

    void drawHealthBar(sf::RenderTarget& target, float yOffset);
};