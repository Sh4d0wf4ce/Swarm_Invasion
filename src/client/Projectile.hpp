#pragma once 

#include "NetworkProtocol.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
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

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    virtual std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map);

    bool isActive() const { return m_active; }
    std::uint32_t getOwnerId() const { return m_ownerId; }
    WeaponType getWeaponType() const { return m_weaponType; }

protected:
    std::uint32_t m_ownerId;
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_lifetime;
    float m_radius;
    bool m_active{true};
    Faction m_faction;
    WeaponType m_weaponType;
};



class RifleProjectile : public Projectile {
public:
    RifleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    sf::CircleShape m_shape;
};

class LaserProjectile : public Projectile {
public:
    LaserProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    sf::CircleShape m_shape;
};

class RocketProjectile : public Projectile {
public:
    RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::vector<std::uint32_t> checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) override;

private:
    sf::CircleShape m_shape;
    float m_explosionRadius{80.0f};
};


class AcidProjectile : public Projectile {
public:
    AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
	sf::CircleShape m_shape;		
};