#include "ProjectileManager.hpp"


void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon, bool isEnemy){
    const auto& stats = WeaponRegistry::getStats(weapon);

    sf::Vector2f direction = targetPos - startPos;
    float length = direction.length();
    if(length != 0) direction /= length;
    sf::Vector2f velocity = direction * stats.speed;

    if(weapon == WeaponType::Rocket){
        m_projectiles.push_back(std::make_unique<RocketProjectile>(ownerId, startPos, velocity, isEnemy));
    }else if (weapon == WeaponType::AcidSpit){
        m_projectiles.push_back(std::make_unique<AcidProjectile>(ownerId, startPos, velocity, isEnemy));
    }else{
        m_projectiles.push_back(std::make_unique<RifleProjectile>(ownerId, startPos, velocity, isEnemy));
    }
}

ProjectileHits ProjectileManager::update(
    sf::Time deltaTime, 
    const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies,
    const std::shared_ptr<MapGenerator>& map,
    std::uint32_t myPlayerId,
    Player* localPlayer)
{
    ProjectileHits allHits;

    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;

        proj->update(deltaTime);

        auto hitsFromThisProj = proj->checkCollisions(enemies, map, localPlayer);

        if(proj->getOwnerId() == myPlayerId && !hitsFromThisProj.hitEnemies.empty()){
            allHits.hitEnemies.insert(allHits.hitEnemies.end(), hitsFromThisProj.hitEnemies.begin(), hitsFromThisProj.hitEnemies.end());
        }

        if(hitsFromThisProj.hitLocalPlayer){
            allHits.hitLocalPlayer = true;
            allHits.damageToPlayer += hitsFromThisProj.damageToPlayer;
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