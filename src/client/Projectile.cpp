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
    
RifleProjectile::RifleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Rifle) {
    
    m_lifetime = 1.5f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void RifleProjectile::update(sf::Time deltaTime){
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();

    if(m_lifetime <= 0.0f) m_active = false;
}

void RifleProjectile::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}


LaserProjectile::LaserProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Laser) {
    
    m_lifetime = 1.5f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void LaserProjectile::update(sf::Time deltaTime){
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();

    if(m_lifetime <= 0.0f) m_active = false;
}

void LaserProjectile::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}


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

AcidProjectile::AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::AcidSpit) {
    m_lifetime = 4.0f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void AcidProjectile::update(sf::Time deltaTime) {
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();
    if(m_lifetime <= 0.0f) m_active = false;
}

void AcidProjectile::render(sf::RenderTarget& target) {
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}