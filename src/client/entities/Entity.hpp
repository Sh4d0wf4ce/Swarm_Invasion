#pragma once

#include "MapGenerator.hpp"
#include "Config.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

/**
 * @brief Identifies which side an entity belongs to in combat.
 */
enum class Faction{
    Players,
    Enemies,
    None,
};

/**
 * @brief Abstract base class for all game-world entities.
 *
 * Stores identity, position, and health state shared by players, enemies,
 * and ability-spawned objects. Subclasses implement per-frame simulation,
 * rendering, faction membership, and collision radius.
 */
class Entity{
public:
    Entity(std::uint32_t id, const sf::Vector2f& startPos): m_id(id), m_position(startPos) {}

    virtual ~Entity() = default;

    virtual void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    
    std::uint32_t getId() const { return m_id; }
    sf::Vector2f getPosition() const { return m_position; }
    float getHp() const { return m_hp; }

    void setPosition(const sf::Vector2f&  pos) { m_position = pos; } 
    void setHp(float hp){ m_hp = hp; }
    void setMaxHp(float maxHp){ m_maxHp = maxHp; }

    
    virtual Faction getFaction() const = 0;
    virtual float getRadius() const = 0;

protected:
    std::uint32_t m_id;
    sf::Vector2f m_position;

    float m_hp{100.0f};
    float m_maxHp{100.0f};

    
    void drawHealthBar(sf::RenderTarget& target, float yOffset);
};
