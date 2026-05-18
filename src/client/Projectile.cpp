#include "Projectile.hpp"

Projectile::Projectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity,  bool isEnemy)
    : m_ownerId(ownerId), m_position(startPos), m_velocity(velocity), m_lifetime(3.0f), m_isEnemyProjectile(isEnemy)  {}



RifleProjectile::RifleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy)
    : Projectile(ownerId, startPos, velocity, isEnemy) {
    
    m_lifetime = 1.5f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(sf::Color::Cyan);
    m_shape.setOrigin({m_radius, m_radius});
    m_weaponType = WeaponType::Rifle;
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

std::vector<std::uint32_t> RifleProjectile::checkCollisions(const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies, const std::shared_ptr<MapGenerator>& map, Player* localPlayer){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    //wall collision
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);

        if(map->getTile(gridX, gridY) == TileType::Wall){
            m_active = false;
            return hitlist;
        }
    }


    if(m_isEnemyProjectile && localPlayer){
        float playerRadius = HeroRegistry::getStats(localPlayer->getClass()).radius;
        sf::Vector2f diff = m_position - localPlayer->getPosition();
        if(diff.lengthSquared() < std::pow(playerRadius + m_radius, 2)){
            m_active = false;
        }
    }else if(!m_isEnemyProjectile){
        //enemy collision
        for(const auto& [enemyId, enemy] : enemies){
            float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;
    
            sf::Vector2f diff = m_position - enemy->getPosition();
            if(diff.lengthSquared() < std::pow(enemyRadius + m_radius, 2)){
                hitlist.push_back(enemyId);
                m_active = false;
                break;
            }
        }
    }

    return hitlist;
}

LaserProjectile::LaserProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy)
    : Projectile(ownerId, startPos, velocity, isEnemy) {
    
    m_lifetime = 1.5f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(sf::Color::Cyan);
    m_shape.setOrigin({m_radius, m_radius});
    m_weaponType = WeaponType::Laser;
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

std::vector<std::uint32_t> LaserProjectile::checkCollisions(const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies, const std::shared_ptr<MapGenerator>& map, Player* localPlayer){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    //wall collision
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);

        if(map->getTile(gridX, gridY) == TileType::Wall){
            m_active = false;
            return hitlist;
        }
    }


    if(m_isEnemyProjectile && localPlayer){
        float playerRadius = HeroRegistry::getStats(localPlayer->getClass()).radius;
        sf::Vector2f diff = m_position - localPlayer->getPosition();
        if(diff.lengthSquared() < std::pow(playerRadius + m_radius, 2)){
            m_active = false;
        }
    }else if(!m_isEnemyProjectile){
        //enemy collision
        for(const auto& [enemyId, enemy] : enemies){
            float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;
    
            sf::Vector2f diff = m_position - enemy->getPosition();
            if(diff.lengthSquared() < std::pow(enemyRadius + m_radius, 2)){
                hitlist.push_back(enemyId);
                m_active = false;
                break;
            }
        }
    }

    return hitlist;
}


RocketProjectile::RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy)
    : Projectile(ownerId, startPos, velocity, isEnemy) {
    
    m_lifetime = 5.0f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(sf::Color::Red);
    m_shape.setOrigin({m_radius, m_radius});
    m_weaponType = WeaponType::Rocket;
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

std::vector<std::uint32_t> RocketProjectile::checkCollisions(const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies, const std::shared_ptr<MapGenerator>& map, Player* localPlayer){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    bool detonated = false;

    //Wall collision
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);

        if(map->getTile(gridX, gridY) == TileType::Wall) detonated = true;
    }


    //Direct enemy collision
    if(!detonated){
        for(const auto& [enemyId, enemy] : enemies){
            float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;

            sf::Vector2f diff = m_position - enemy->getPosition();
            float collDist = enemyRadius + m_radius;
            if(diff.lengthSquared() < collDist * collDist){
                detonated = true;
                break;
            }
        }
    }

    //Rocket explosion
    if(detonated){
        for(const auto& [enemyId, enemy] : enemies){
            sf::Vector2f diff = m_position - enemy->getPosition();
            
            if(diff.lengthSquared() <= m_explosionRadius * m_explosionRadius){
                hitlist.push_back(enemyId);
            }
        }
        m_active = false;
    }

    return hitlist;
}

AcidProjectile::AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, bool isEnemy)
    : Projectile(ownerId, startPos, velocity, isEnemy) {
    m_lifetime = 4.0f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(sf::Color::Green);
    m_shape.setOrigin({m_radius, m_radius});
    m_weaponType = WeaponType::AcidSpit;
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

std::vector<std::uint32_t> AcidProjectile::checkCollisions(const std::map<uint32_t, std::unique_ptr<Enemy>>& enemies, const std::shared_ptr<MapGenerator>& map, Player* localPlayer){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if(map->getTile(gridX, gridY) == TileType::Wall){
            m_active = false;
            return hitlist;
        }
    }

    // Enemy projectile
    if (m_isEnemyProjectile && localPlayer) {
        float playerRadius = HeroRegistry::getStats(localPlayer->getClass()).radius;
        sf::Vector2f diff = m_position - localPlayer->getPosition();
        if(diff.lengthSquared() < std::pow(playerRadius + m_radius, 2)){
            hitlist.push_back(localPlayer->getId());
            m_active = false;
        }
    }
    // Player Projectile
    else if(!m_isEnemyProjectile){
        for(const auto& [enemyId, enemy] : enemies){
            float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;
            sf::Vector2f diff = m_position - enemy->getPosition();
            if(diff.lengthSquared() < std::pow(enemyRadius + m_radius, 2)){
                hitlist.push_back(enemyId);
                m_active = false;
                break;
            }
        }
    }

    return hitlist;
}