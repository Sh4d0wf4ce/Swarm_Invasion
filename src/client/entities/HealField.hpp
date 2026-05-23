#pragma once

#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class HealField : public Entity{
public:
    HealField(std::uint32_t id, const sf::Vector2f& startPos, float duration, float radius) : Entity(id, startPos), m_radius(radius){
        m_hp = duration;
        m_maxHp = duration;

        m_shape.setRadius(radius);
        m_shape.setOrigin({radius, radius});
        m_shape.setPosition(startPos);
        m_shape.setFillColor(sf::Color(0, 255, 0, 50));
        m_shape.setOutlineColor(sf::Color(0, 255, 0, 150));
        m_shape.setOutlineThickness(2.0f);
    }

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) { m_hp -= deltaTime.asSeconds(); } ;
    void render(sf::RenderTarget& target) { target.draw(m_shape); };

    Faction getFaction() const override { return Faction::Players; }
    float getRadius() const override { return m_radius; }

    float getHealPerSecond() const { return m_healPerSecond; }

private:
    float m_healPerSecond{10.0f};
    float m_radius;
    sf::CircleShape m_shape;
};