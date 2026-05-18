#include "RocketProjectile.hpp"

RocketProjectile::RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Rocket) {
    
    m_lifetime = 5.0f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void RocketProjectile::update(sf::Time deltaTime){
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();

    if(m_lifetime <= 0.0f) m_active = false;
}

void RocketProjectile::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}

std::vector<std::uint32_t> RocketProjectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    bool detonated = false;

    //Wall collision
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);

        if(map->getTile(gridX, gridY) == TileType::Wall) detonated = true;
    }


    //Direct entity collision
    if(!detonated){
        for(Entity* entity : entities){
            if(entity->getFaction() == m_faction) continue;
            if(entity->getId() == m_ownerId) continue;

            sf::Vector2f diff = m_position - entity->getPosition();
            float collDist = entity->getRadius() + m_radius;
            if(diff.lengthSquared() < collDist * collDist){
                detonated = true;
                break;
            }
        }
    }

    //Rocket explosion
    if(detonated){
        for(Entity* entity : entities){
            if(entity->getFaction() == m_faction) continue;
            if(entity->getId() == m_ownerId) continue;

            sf::Vector2f diff = m_position - entity->getPosition();
            
            if(diff.lengthSquared() <= m_explosionRadius * m_explosionRadius){
                hitlist.push_back(entity->getId());
            }
        }
        m_active = false;
    }

    return hitlist;
}