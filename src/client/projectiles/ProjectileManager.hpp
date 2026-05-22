#pragma once

#include "Projectile.hpp"
#include "../entities/Player.hpp"
#include "WeaponRegistry.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>
#include <memory>

struct HitRecord {
    std::uint32_t shooterId;
    std::uint32_t targetId;
    WeaponType weapon;
};

class ProjectileManager{
public:
    ProjectileManager() = default;

    void spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction);

    std::vector<HitRecord> update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map);
    void render(sf::RenderTarget& target);

private:
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
};