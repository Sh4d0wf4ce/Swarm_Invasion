#include "ProjectileManager.hpp"
#include "RocketProjectile.hpp"


void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction){
    const auto& stats = WeaponRegistry::getStats(weapon);
    
    sf::Vector2f direction = targetPos - startPos;
    float baseAngle = std::atan2(direction.y, direction.x);

    float spreadRad = stats.spreadAngle * (M_PI / 180.0f); 
    float startAngle = baseAngle - (spreadRad * (stats.pellets - 1) / 2.0f);

    for (int i = 0; i < stats.pellets; ++i) {
        float currentAngle = startAngle + (i * spreadRad);
        
        sf::Vector2f velocity(std::cos(currentAngle) * stats.speed, std::sin(currentAngle) * stats.speed);
        
        if(weapon == WeaponType::Rocket){
            m_projectiles.push_back(std::make_unique<RocketProjectile>(ownerId, startPos, velocity, faction));
        } else {
            m_projectiles.push_back(std::make_unique<Projectile>(ownerId, startPos, velocity, faction, weapon));
        }
    }
}

std::vector<HitRecord> ProjectileManager::update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map){
    std::vector<HitRecord> allHits;

    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->update(deltaTime);

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