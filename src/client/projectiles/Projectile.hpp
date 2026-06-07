#pragma once 
#include "NetworkProtocol.hpp"
#include "../entities/Enemy.hpp"
#include "../entities/Player.hpp"
#include "MapGenerator.hpp"
#include "WeaponRegistry.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <map>
#include <vector>

class Projectile {
public:
    Projectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction, WeaponType weapon);
    virtual ~Projectile() = default;
    virtual void update(sf::Time deltaTime);
    virtual void render(sf::RenderTarget& target);

    virtual std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map);


    bool isActive() const { return m_active; }
    std::uint32_t getOwnerId() const { return m_ownerId; }
    WeaponType getWeaponType() const { return m_weaponType; }
    sf::Vector2f getPosition() const { return m_position; }
    float getRadius() const { return m_radius; }
    Faction getFaction() const { return m_faction; }
    void deactivate() { m_active = false; }

protected:
    sf::CircleShape m_shape;

    std::uint32_t m_ownerId;
    Faction m_faction;
    WeaponType m_weaponType;

    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_lifetime;
    float m_radius;
    bool m_active{true};
};
