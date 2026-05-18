#include "Projectile.hpp"

Projectile::Projectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity,  Faction faction, WeaponType weapon)
    : m_ownerId(ownerId), m_position(startPos), m_velocity(velocity), m_faction(faction), m_weaponType(weapon)  {

        const auto& stats = WeaponRegistry::getStats(weapon);
        m_radius = stats.radius;
        m_lifetime = stats.lifetime;
        m_active = true;
}


std::vector<std::uint32_t> Projectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    // Projectile hits a wall
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if(map->getTile(gridX, gridY) == TileType::Wall){
            m_active = false;
            return hitlist;
        }
    }

    // Projectile hits an entity
    for(Entity* entity : entities) {
        if(entity->getFaction() == m_faction) continue;
        if(entity->getId() == m_ownerId) continue;

        sf::Vector2f diff = m_position - entity->getPosition();
        float collDist = entity->getRadius() + m_radius;

        if(diff.lengthSquared() < collDist * collDist){
            hitlist.push_back(entity->getId());
            m_active = false;
            break;
        }
    }

    return hitlist;
}