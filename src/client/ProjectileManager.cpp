#include "ProjectileManager.hpp"

ProjectileManager::ProjectileManager(){
    m_projectileShape.setRadius(5.0f);
    m_projectileShape.setFillColor(sf::Color::Yellow);
    m_projectileShape.setOrigin({5.0f, 5.0f});

    m_projectiles.reserve(1000);
}

void ProjectileManager::spawnProjectile(const sf::Vector2f& startPos, const sf::Vector2f& targetPos, float speed){
    sf::Vector2f direction = targetPos - startPos;
    float length = direction.length();

    if(length != 0){
        direction /= length;
    }
    direction *= speed;

    for(auto& proj: m_projectiles){
        if(!proj.active){
            proj.position = startPos;
            proj.velocity = direction;
            proj.lifetime = 3.0f;
            proj.active = true;
            return;
        }
    }

    m_projectiles.push_back({startPos, direction, 3.0f, true});
}

std::vector<std::uint32_t> ProjectileManager::update(
    sf::Time deltaTime, 
    const std::map<std::uint32_t, std::unique_ptr<Player>>& enemies,
    const std::shared_ptr<MapGenerator>& map)
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
            int gridX = static_cast<int>(proj.position.x / 32.0f);
            int gridY = static_cast<int>(proj.position.y / 32.0f);

            if(map->getTile(gridX, gridY) == TileType::Wall){
                proj.active = false;
                continue;
            }
        }

        // Collision with enemies
        for(const auto& [id, enemy] : enemies){
            sf::Vector2f diff = proj.position - enemy->getPosition();
            float distSq = diff.lengthSquared();

            if(distSq < 625.0f){
                hitEnemies.push_back(id);
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