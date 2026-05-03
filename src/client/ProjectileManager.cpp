#include "ProjectileManager.hpp"


void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, WeaponType weapon){
    const auto& stats = WeaponRegistry::getStats(weapon);

    sf::Vector2f direction = targetPos - startPos;
    float length = direction.length();
    if(length != 0) direction /= length;
    sf::Vector2f velocity = direction * stats.speed;

    if(weapon == WeaponType::Rocket){
        m_projectiles.push_back(std::make_unique<RocketProjectile>(ownerId, startPos, velocity));
    }else{
        m_projectiles.push_back(std::make_unique<RifleProjectile>(ownerId, startPos, velocity));
    }
}

std::vector<std::uint32_t> ProjectileManager::update(
    sf::Time deltaTime, 
    const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies,
    const std::shared_ptr<MapGenerator>& map,
    std::uint32_t myPlayerId)
{
    std::vector<std::uint32_t> allHits;

    for(auto& proj: m_projectiles){
        if(!proj->isActive()) continue;

        proj->update(deltaTime);

        auto hitsFromThisProj = proj->checkCollisions(enemies, map);

        if(proj->getOwnerId() == myPlayerId && !hitsFromThisProj.empty()){
            allHits.insert(allHits.end(), hitsFromThisProj.begin(), hitsFromThisProj.end());
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