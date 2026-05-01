#pragma once

#include "Player.hpp"
#include "MapGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>


struct Projectile{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    bool active;
};

class ProjectileManager{
public:
    ProjectileManager();

    void spawnProjectile(const sf::Vector2f& startPos, const sf::Vector2f& targetPos, float speed);

    std::vector<std::uint32_t> update(
        sf::Time deltaTime, 
        const std::map<std::uint32_t, std::unique_ptr<Player>>& enemies,
        const std::shared_ptr<MapGenerator>& map
    );
    void render(sf::RenderTarget& target);

private:
    std::vector<Projectile> m_projectiles;
    sf::CircleShape m_projectileShape;
};