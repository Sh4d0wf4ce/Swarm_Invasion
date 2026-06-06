#include "ProjectileManager.hpp"
#include "RocketProjectile.hpp"
#include "ShurikenProjectile.hpp"
#include "WaveProjectile.hpp"
#include "NeedleProjectile.hpp"
#include "DroneBlasterProjectile.hpp"


void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction){
    const auto& stats = WeaponRegistry::getStats(weapon);
    
    sf::Vector2f direction = targetPos - startPos;
    float lenSq = direction.lengthSquared();
    if(lenSq <= 0.0f) return;

    float baseAngle = std::atan2(direction.y, direction.x);
    float spreadRad = stats.spreadAngle * (M_PI / 180.0f);

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

std::vector<HitRecord> ProjectileManager::update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map, const std::vector<SectorBarrierSnapshot>& barriers){
    std::vector<HitRecord> allHits;

    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->update(deltaTime);

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

        auto hitIds = proj->checkCollisions(entities, map);
        for(std::uint32_t id: hitIds){
            allHits.push_back({proj->getOwnerId(), id, proj->getWeaponType()});
        }
    }

    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(), 
            [](const std::unique_ptr<Projectile>& p) { return !p->isActive();}),
            m_projectiles.end()
    );

    return allHits;
}

void ProjectileManager::render(sf::RenderTarget& target){
    for(const auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->render(target);
    }
}