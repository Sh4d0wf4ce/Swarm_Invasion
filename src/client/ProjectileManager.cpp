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

void ProjectileManager::update(sf::Time deltaTime){
    float dt = deltaTime.asSeconds();

    for(auto& proj: m_projectiles){
        if(!proj.active) continue;

        proj.position += proj.velocity * dt;
        proj.lifetime -= dt;

        if(proj.lifetime < 0.0f){
            proj.active = false;
        }
    }
}

void ProjectileManager::render(sf::RenderTarget& target){
    for(const auto& proj: m_projectiles){
        if(!proj.active) continue;

        m_projectileShape.setPosition(proj.position);
        target.draw(m_projectileShape);
    }
}