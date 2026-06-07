#include "ProjectileManager.hpp"
#include "RocketProjectile.hpp"
#include "ShurikenProjectile.hpp"
#include "WaveProjectile.hpp"
#include "NeedleProjectile.hpp"
#include "DroneBlasterProjectile.hpp"

/**
 * @brief Creates one or more projectiles aimed at a target with optional pellet spread.
 * @param ownerId Entity ID of the shooter.
 * @param startPos World position where projectiles spawn.
 * @param targetPos Aim point used to compute direction.
 * @param weapon Weapon type determining stats and derived projectile class.
 * @param faction Faction assigned to spawned projectiles.
 */
void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction){
    const auto& stats = WeaponRegistry::getStats(weapon);

    // --- Compute base aim direction and reject zero-length shots ---
    sf::Vector2f direction = targetPos - startPos;
    float lenSq = direction.lengthSquared();
    if(lenSq <= 0.0f) return;
    float baseAngle = std::atan2(direction.y, direction.x);
    float spreadRad = stats.spreadAngle * (M_PI / 180.0f);

    // --- Spawn one projectile per pellet with optional spread ---
    for (int i = 0; i < stats.pellets; ++i) {
        float currentAngle = baseAngle;
        if(stats.pellets > 1){
            float randomOffset = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * spreadRad;
            currentAngle += randomOffset;
        }

        sf::Vector2f velocity(std::cos(currentAngle) * stats.speed, std::sin(currentAngle) * stats.speed);
        if(weapon == WeaponType::Rocket){
            m_projectiles.push_back(std::make_unique<RocketProjectile>(ownerId, startPos, velocity, faction));
        } 
        else if (weapon == WeaponType::Shuriken) {
            m_projectiles.push_back(std::make_unique<ShurikenProjectile>(ownerId, startPos, velocity, faction));
        } 
        else if (weapon == WeaponType::VanguardWave) {
            m_projectiles.push_back(std::make_unique<WaveProjectile>(ownerId, startPos, velocity, faction));
        }
        else if (weapon == WeaponType::MedicNeedle) {
            m_projectiles.push_back(std::make_unique<NeedleProjectile>(ownerId, startPos, velocity, faction));
        }
        else if (weapon == WeaponType::DroneBlaster) {
            m_projectiles.push_back(std::make_unique<DroneBlasterProjectile>(ownerId, startPos, velocity, faction));
        }
        else {
            m_projectiles.push_back(std::make_unique<Projectile>(ownerId, startPos, velocity, faction, weapon));
        }
    }
}

/**
 * @brief Updates all projectiles, applies barrier blocking, and collects local hits.
 * @param deltaTime Elapsed time since the last update.
 * @param entities Entities available for collision tests.
 * @param map Map used for wall collision checks.
 * @param barriers Active medic barrier snapshots that block enemy projectiles.
 * @param localPlayerId Local player ID; only their shots generate hit records.
 * @return Hit records for projectiles owned by the local player.
 */
std::vector<HitRecord> ProjectileManager::update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map, const std::vector<SectorBarrierSnapshot>& barriers, std::uint32_t localPlayerId){
    std::vector<HitRecord> allHits;

    // --- Update each active projectile and collect hits ---
    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->update(deltaTime);

        // --- Block enemy projectiles against medic barriers ---
        if (proj->getFaction() == Faction::Enemies) {
            for (const auto& barrier : barriers) {
                if (SectorMath::isInArcWall(
                        barrier.center,
                        barrier.facingAngle,
                        barrier.arcRadius,
                        barrier.halfSpan,
                        barrier.wallThickness,
                        proj->getPosition(),
                        proj->getRadius())) {
                    proj->deactivate();
                    break;
                }
            }
            if (!proj->isActive()) continue;
        }
        const bool ownsProjectile = localPlayerId == 0 || proj->getOwnerId() == localPlayerId;
        const WeaponType weapon = proj->getWeaponType();
        const bool visualHealCollision = weapon == WeaponType::MedicNeedle || weapon == WeaponType::DroneBlaster;
        const bool checkEntities = ownsProjectile || visualHealCollision;
        auto hitIds = proj->checkCollisions(checkEntities ? entities : std::vector<Entity*>{}, map);
        if (!ownsProjectile) continue;
        for(std::uint32_t id: hitIds){
            allHits.push_back({proj->getOwnerId(), id, proj->getWeaponType()});
        }
    }

    // --- Remove inactive projectiles ---
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(), 
            [](const std::unique_ptr<Projectile>& p) { return !p->isActive();}),
        m_projectiles.end()
    );
    return allHits;
}

/**
 * @brief Renders all active projectiles.
 * @param target Render target to draw into.
 */
void ProjectileManager::render(sf::RenderTarget& target){
    for(const auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->render(target);
    }
}
