#include "ProjectileManager.hpp"

ProjectileManager::ProjectileManager(){
    m_projectileShape.setRadius(Config::PROJECTILE_RADIUS);
    m_projectileShape.setFillColor(sf::Color::Yellow);
    m_projectileShape.setOrigin({Config::PROJECTILE_RADIUS, Config::PROJECTILE_RADIUS});

    m_projectiles.reserve(1000);
}

void ProjectileManager::spawnProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& targetPos, float speed){
    sf::Vector2f direction = targetPos - startPos;
    float length = direction.length();

    if(length != 0){
        direction /= length;
    }
    direction *= speed;

    for(auto& proj: m_projectiles){
        if(!proj.active){
            proj.ownerId = ownerId;
            proj.position = startPos;
            proj.velocity = direction;
            proj.lifetime = Config::PROJECTILE_LIFETIME;
            proj.active = true;
            return;
        }
    }

    m_projectiles.push_back({ownerId, startPos, direction, Config::PROJECTILE_LIFETIME, true});
}

std::vector<std::uint32_t> ProjectileManager::update(
    sf::Time deltaTime, 
    const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies,
    const std::shared_ptr<MapGenerator>& map,
    std::uint32_t myPlayerId)
{
    float dt = deltaTime.asSeconds();
    std::vector<std::uint32_t> hitEnemies;

    for(auto& proj: m_projectiles){
        if(!proj.active) continue;

        proj.position += proj.velocity * dt;
        proj.lifetime -= dt;

        if(proj.lifetime < 0.0f){
            proj.active = false;
            continue;
        }

        // Collision with walls
        if(map){
            int gridX = static_cast<int>(proj.position.x / Config::TILE_SIZE);
            int gridY = static_cast<int>(proj.position.y / Config::TILE_SIZE);

            if(map->getTile(gridX, gridY) == TileType::Wall){
                proj.active = false;
                continue;
            }
        }

        // Collision with enemies
        for(const auto& [id, enemy] : enemies){
            sf::Vector2f diff = proj.position - enemy->getPosition();
            float collisionDist = Config::ENEMY_RADIUS + Config::PROJECTILE_RADIUS;

            if(diff.lengthSquared() < collisionDist * collisionDist){
                if(proj.ownerId == myPlayerId){
                    hitEnemies.push_back(id);
                }
                proj.active = false;
                break;
            }
        }
    }

    return hitEnemies;
}

void ProjectileManager::render(sf::RenderTarget& target){
    for(const auto& proj: m_projectiles){
        if(!proj.active) continue;

        m_projectileShape.setPosition(proj.position);
        target.draw(m_projectileShape);
    }
}