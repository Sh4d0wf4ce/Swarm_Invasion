#pragma once

#include "Projectile.hpp"
#include "WeaponRegistry.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>
#include <memory>

class ProjectileManager{
public:
    ProjectileManager() = default;

    void spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon);

    std::vector<std::uint32_t> update(
        sf::Time deltaTime, 
        const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies,
        const std::shared_ptr<MapGenerator>& map,
        std::uint32_t myPLayerId
    );
    void render(sf::RenderTarget& target);

private:
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
};