#pragma once

#include "Enemy.hpp"
#include "MapGenerator.hpp"
#include "Config.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>


struct Projectile{
    std::uint32_t ownerId;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    bool active;
};

class ProjectileManager{
public:
    ProjectileManager();

    void spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, float speed);

    std::vector<std::uint32_t> update(
        sf::Time deltaTime, 
        const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies,
        const std::shared_ptr<MapGenerator>& map,
        std::uint32_t myPLayerId
    );
    void render(sf::RenderTarget& target);

private:
    std::vector<Projectile> m_projectiles;
    sf::CircleShape m_projectileShape;
};