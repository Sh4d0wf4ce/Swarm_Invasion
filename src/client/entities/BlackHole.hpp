#pragma once

#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class BlackHole : public Entity{
public:
    BlackHole(std::uint32_t id, const sf::Vector2f& startPos, float duration) : Entity(id, startPos){
        m_hp = duration;
        m_maxHp = duration;

        m_coreShape.setRadius(30.0f);
        m_coreShape.setOrigin({30.0f, 30.0f});
        m_coreShape.setPosition(m_position);
        m_coreShape.setFillColor(sf::Color(20, 20, 20));
        
        m_pullShape.setRadius(300.0f);
        m_pullShape.setOrigin({300.0f, 300.0f});
        m_pullShape.setPosition(m_position);
        m_pullShape.setFillColor(sf::Color::Transparent);    
        m_pullShape.setOutlineThickness(3.0f);
    }

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
        m_hp -= deltaTime.asSeconds(); 
        
        float pulse = std::abs(std::sin(m_hp * 5.0f));
        m_pullShape.setOutlineColor(sf::Color(138, 43, 226, static_cast<std::uint8_t>(100 + 50 * pulse)));
    }

    void render(sf::RenderTarget& target){
        target.draw(m_pullShape);
        target.draw(m_coreShape);
    };

    
    Faction getFaction() const override { return Faction::Players; }
    float getRadius() const override { return 300.0f; }

private:
    sf::CircleShape m_coreShape;
    sf::CircleShape m_pullShape;
};
