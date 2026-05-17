#pragma once 

#include "Enemy.hpp"
#include "Player.hpp"
#include "MapGenerator.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <map>
#include <vector>

struct ProjectileHits{
    std::vector<std::uint32_t> hitEnemies;
    bool hitLocalPlayer = false;
    float damageToPlayer = 0.0f;
};

class Projectile {
public:
    Projectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy);
    virtual ~Projectile() = default;

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    virtual ProjectileHits checkCollisions(
        const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies,
        const std::shared_ptr<MapGenerator>& map,
        Player* localPlayer
    ) = 0;

    bool isActive() const { return m_active; }
    std::uint32_t getOwnerId() const { return m_ownerId; }

protected:
    std::uint32_t m_ownerId;
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;
    float m_lifetime;
    bool m_active{true};
    bool m_isEnemyProjectile;
};



class RifleProjectile : public Projectile {
public:
    RifleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    ProjectileHits checkCollisions(
            const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies,
            const std::shared_ptr<MapGenerator>& map,
            Player* localPlayer
        ) override;

private:
        sf::CircleShape m_shape;
        float m_radius{5.0f};
};

class RocketProjectile : public Projectile {
public:
    RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    ProjectileHits checkCollisions(
            const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies,
            const std::shared_ptr<MapGenerator>& map,
            Player* localPlayer
        ) override;

private:
        sf::CircleShape m_shape;
        float m_radius{5.0f};
        float m_explosionRadius{80.0f};
};


class AcidProjectile : public Projectile {
public:
    AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy);

    void update(sf::Time deltaTime) override;
    void render(sf::RenderTarget& target) override;

    ProjectileHits checkCollisions(
            const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies,
            const std::shared_ptr<MapGenerator>& map,
            Player* localPlayer
        ) override;

private:
	sf::CircleShape m_shape;
	float m_radius{5.0f};		
};