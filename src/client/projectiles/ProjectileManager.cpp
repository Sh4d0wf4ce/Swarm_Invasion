#include "ProjectileManager.hpp"
#include "LaserProjectile.hpp"
#include "AcidProjectile.hpp"
#include "RifleProjectile.hpp"
#include "RocketProjectile.hpp"


void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, Faction faction){
    const auto& stats = WeaponRegistry::getStats(weapon);

    sf::Vector2f direction = targetPos - startPos;
    float length = direction.length();
    if(length != 0) direction /= length;
    sf::Vector2f velocity = direction * stats.speed;

    if(weapon == WeaponType::Rocket){
        m_projectiles.push_back(std::make_unique<RocketProjectile>(ownerId, startPos, velocity, faction));
    }else if (weapon == WeaponType::AcidSpit){
        m_projectiles.push_back(std::make_unique<AcidProjectile>(ownerId, startPos, velocity, faction));
    }else if (weapon == WeaponType::Laser){
        m_projectiles.push_back(std::make_unique<LaserProjectile>(ownerId, startPos, velocity, faction));
    }else{
        m_projectiles.push_back(std::make_unique<RifleProjectile>(ownerId, startPos, velocity, faction));
    }
}

std::vector<HitRecord> ProjectileManager::update(sf::Time deltaTime, const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map){
    std::vector<HitRecord> allHits;

    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;
        proj->update(deltaTime);

        auto hitIds = proj->checkCollisions(entities, map);
        for(std::uint32_t id: hitIds){
            allHits.push_back({id, proj->getWeaponType()});
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