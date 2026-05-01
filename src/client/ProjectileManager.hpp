#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

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

    void update(sf::Time deltaTime);
    void render(sf::RenderTarget& target);

private:
    std::vector<Projectile> m_projectiles;
    sf::CircleShape m_projectileShape;
};