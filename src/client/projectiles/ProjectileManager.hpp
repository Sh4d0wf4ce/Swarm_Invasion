#pragma once
#include "Projectile.hpp"
#include "../entities/Player.hpp"
#include "WeaponRegistry.hpp"
#include "SectorMath.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>
#include <memory>

// ==========================================
// Collision & Barrier Data
// ==========================================

/**
 * @brief Records a confirmed hit between a shooter and a target.
 */
struct HitRecord {
    std::uint32_t shooterId;
    std::uint32_t targetId;
    WeaponType weapon;
};

/**
 * @brief Snapshot of a medic barrier arc used for server-side collision blocking on the client.
 */
struct SectorBarrierSnapshot {
    sf::Vector2f center;
    float facingAngle;
    float arcRadius;
    float halfSpan;
    float wallThickness;
};

/**
 * @brief Spawns, updates, and renders all active client projectiles.
 *
 * Factory-selects derived projectile types from weapon enum values and collects
 * hit records for locally owned shots.
 */
class ProjectileManager{
public:
    ProjectileManager() = default;

    void spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction);

    std::vector<HitRecord> update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map, const std::vector<SectorBarrierSnapshot>& barriers = {}, std::uint32_t localPlayerId = 0);
    void render(sf::RenderTarget& target);
    
private:
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
};
