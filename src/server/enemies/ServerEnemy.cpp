#include "ServerEnemy.hpp"
#include "Config.hpp"
#include "EnemyRegistry.hpp"
#include "HeroRegistry.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>

ServerEnemy::ServerEnemy(std::uint32_t id, const sf::Vector2f& startPos, EnemyType type): m_id(id), m_position(startPos), m_type(type) {
    const auto& stats = EnemyRegistry::getStats(type);
    m_hp = stats.maxHp;
    m_speed = stats.speed;
}

bool ServerEnemy::hasLineOfSight(sf::Vector2f start, sf::Vector2f end, float radius, std::shared_ptr<MapGenerator> map){
    sf::Vector2f dir = end - start;
    float dist = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if(dist == 0) return true;
    dir /= dist;
    float step = Config::TILE_SIZE / 2.0f;
    for(float d = 0; d < dist; d += step){
        if(map->checkCollision(start + dir * d, radius)) return false;
    }
    return true;
}

sf::Vector2f ServerEnemy::calculateSeparation(const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, float myRadius, const SpatialGrid& grid){
    sf::Vector2f separationVector(0.0f, 0.0f);
    std::vector<std::uint32_t> nearby = grid.getNearby(m_position);

    for(std::uint32_t otherId : nearby){
        if(m_id == otherId) continue;
        auto it = allEnemies.find(otherId);
        
        if(it != allEnemies.end()){
            const auto& otherEnemy = it->second;
            
            sf::Vector2f diff = m_position - otherEnemy->getPosition();
            float distSq = diff.lengthSquared();
            float combinedRadius = myRadius + EnemyRegistry::getStats(otherEnemy->getType()).radius;
            
            if(distSq > 0 && distSq < combinedRadius * combinedRadius){
                separationVector += diff / std::sqrt(distSq);
            }
        }
    }
    return separationVector;
}

std::vector<std::uint32_t> ServerEnemy::performMeleeChase(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid) {
    if(m_knockbackVelocity.lengthSquared() > 10.0f){
        sf::Vector2f velocity = m_knockbackVelocity * deltaTime.asSeconds();
        
        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        if(!map->checkCollision(nextPosX, EnemyRegistry::getStats(m_type).radius)) m_position.x = nextPosX.x;
        
        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        if(!map->checkCollision(nextPosY, EnemyRegistry::getStats(m_type).radius)) m_position.y = nextPosY.y;

        m_knockbackVelocity -= m_knockbackVelocity * 6.0f * deltaTime.asSeconds(); 
        
        return {};
    }
    
    std::vector<std::uint32_t> deadPlayers;
    const auto& eStats = EnemyRegistry::getStats(m_type);

    std::uint32_t targetId = clients.begin()->first;
    float minDistanceSq  = (clients.begin()->second.position - m_position).lengthSquared();

    for(auto& [playerId, playerInfo] : clients){
        if(playerInfo.invTimer > 0.0f) continue;
        float distSq = (playerInfo.position - m_position).lengthSquared();
        if(distSq < minDistanceSq){
            minDistanceSq = distSq;
            targetId = playerId;
        }

        float touchDist = eStats.radius + HeroRegistry::getStats(playerInfo.pClass).radius;
        if(distSq < touchDist * touchDist){
            if(m_lastAttackTime.getElapsedTime().asSeconds() > eStats.attackCooldown){
                playerInfo.hp -= eStats.damage;
                m_lastAttackTime.restart();
                if(playerInfo.hp <= 0.0f) deadPlayers.push_back(playerId);
            }
        }   
    }

    sf::Vector2f desiredDirection(0.0f, 0.0f);
    if(hasLineOfSight(m_position, clients.at(targetId).position, eStats.radius, map)){
        desiredDirection = clients.at(targetId).position - m_position;
    } else if(!flowField.empty()){
        int ex = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int ey = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if(ex > 0 && ex < map->getWidth() - 1 && ey > 0 && ey < map->getHeight() - 1){
            int minCost = flowField[ex][ey];
            sf::Vector2f bestTarget = m_position;
            for(int dx = -1; dx <= 1; dx++){
                for(int dy = -1; dy <= 1; dy++){
                    if(dx == 0 && dy == 0) continue;
                    int nx = ex + dx;
                    int ny = ey + dy;
                    if(flowField[nx][ny] < minCost){
                        minCost = flowField[nx][ny];
                        bestTarget = sf::Vector2f((nx + 0.5f) * Config::TILE_SIZE, (ny + 0.5f) * Config::TILE_SIZE);
                    }
                }
            }
            desiredDirection = bestTarget - m_position;
        }
    }

    sf::Vector2f separationVector = calculateSeparation(allEnemies, eStats.radius, grid);
    if(desiredDirection.lengthSquared() > 0) desiredDirection /= std::sqrt(desiredDirection.lengthSquared());
    
    sf::Vector2f finalDirection = desiredDirection + (separationVector * 1.5f);
    float lenSq = finalDirection.lengthSquared();

    if(lenSq > 0){
        finalDirection /= std::sqrt(lenSq);
        sf::Vector2f velocity = finalDirection * m_speed * deltaTime.asSeconds();
        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        if(!map->checkCollision(nextPosX, eStats.radius)) m_position.x = nextPosX.x;
        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        if(!map->checkCollision(nextPosY, eStats.radius)) m_position.y = nextPosY.y;
    }
    return deadPlayers;
}

std::uint32_t ServerEnemy::getClosestPlayerId(const std::map<std::uint32_t, ClientInfo>& clients, float& outMinDistSq) const {
    std::uint32_t targetId = clients.begin()->first;
    outMinDistSq = (clients.begin()->second.position - m_position).lengthSquared();

    for(const auto& [playerId, playerInfo] : clients){
        float distSq = (playerInfo.position - m_position).lengthSquared();
        if(distSq < outMinDistSq){
            outMinDistSq = distSq;
            targetId = playerId;
        }
    }
    return targetId;
}

void ServerEnemy::applyKnockback(sf::Vector2f direction, float force){
    float lenSq = direction.lengthSquared();
    if(lenSq > 0.0f){
        m_knockbackVelocity = (direction / std::sqrt(lenSq)) * force;
    }
}

void ServerEnemy::applyPoison(float duration, float damagePerSecond){
    m_poisonEffects.push_back({duration, damagePerSecond});
}

void ServerEnemy::tickStatusEffects(float dt){
    for (auto& poison : m_poisonEffects) {
        poison.remainingTime -= dt;
        m_hp -= poison.damagePerSecond * dt;
    }

    m_poisonEffects.erase(
        std::remove_if(m_poisonEffects.begin(), m_poisonEffects.end(),
            [](const PoisonEffect& p) { return p.remainingTime <= 0.0f; }),
        m_poisonEffects.end()
    );
}