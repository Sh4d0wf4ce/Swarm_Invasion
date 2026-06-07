#pragma once

#include "Entity.hpp"
#include <SFML/Graphics.hpp>

/**
 * @brief Soldier heal field zone that restores health over its lifetime.
 *
 * Renders a green circle and counts down remaining duration
 * through the entity HP value. Belongs to the player faction.
 */
class HealField : public Entity{
public:
    /**
     * @brief Constructs a heal field with the given duration and radius.
     * @param id Unique network entity identifier.
     * @param startPos Center position in world space.
     * @param duration Total lifetime in seconds stored as HP.
     * @param radius Effect radius in world units.
     */
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

    /**
     * @brief Reduces remaining lifetime each frame.
     * @param deltaTime Elapsed time since the last frame.
     * @param map Tile map reference.
     */
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) { m_hp -= deltaTime.asSeconds(); } ;

    /**
     * @brief Draws the heal field circle.
     * @param target Render target to draw into.
     */
    void render(sf::RenderTarget& target) { target.draw(m_shape); };

    /**
     * @brief Returns the player faction for combat filtering.
     * @return Faction::Players.
     */
    Faction getFaction() const override { return Faction::Players; }

    /**
     * @brief Returns the heal zone collision radius.
     * @return Effect radius in world units.
     */
    float getRadius() const override { return m_radius; }

    /**
     * @brief Returns the healing rate applied to allies inside the field.
     * @return Hit points restored per second.
     */
    float getHealPerSecond() const { return m_healPerSecond; }

private:
    float m_healPerSecond{10.0f};

    float m_radius;
    sf::CircleShape m_shape;
};
