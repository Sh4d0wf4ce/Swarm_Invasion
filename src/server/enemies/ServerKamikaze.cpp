#include "ServerKamikaze.hpp"
#include "HeroRegistry.hpp"

std::vector<std::uint32_t> ServerKamikaze::update(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, std::vector<EnemyShootEvent>& outShootEvents, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies) {
    std::vector<std::uint32_t> deadPlayers;
    if(clients.empty() || m_exploded) return deadPlayers;

    const auto& eStats = EnemyRegistry::getStats(m_type);

    float minDistanceSq;
    std::uint32_t targetId = getClosestPlayerId(clients, minDistanceSq);

    float explosionRadius = eStats.radius + 20.0f;
    if(minDistanceSq < explosionRadius * explosionRadius){
        m_exploded = true;

        for(auto& [playerId, playerInfo] : clients){
            float distSq = (playerInfo.position - m_position).lengthSquared();
            float touchDist = explosionRadius + HeroRegistry::getStats(playerInfo.pClass).radius;
            if(distSq < touchDist * touchDist){
                playerInfo.hp -= eStats.damage;
                if(playerInfo.hp <= 0.0f) deadPlayers.push_back(playerId);
            }
        }

        takeDamage(9999.0f);
        return deadPlayers;
    }

    if(!hasLineOfSight(m_position, clients.at(targetId).position, eStats.radius, map)){
        return performMeleeChase(deltaTime, clients, map, flowField, allEnemies);
    }

    sf::Vector2f dir = clients.at(targetId).position - m_position;
    float len = dir.length();
    if(len > 0.0f) dir /= len;

    sf::Vector2f perpendicular(-dir.y, dir.x);
    float zigzagFreq = 8.0f;
    float zigzagAmp = 500.0f;
    float timeSec = m_aliveTimer.getElapsedTime().asSeconds();

    sf::Vector2f velocity = (dir * m_speed) + (perpendicular * zigzagAmp * std::sin(timeSec * zigzagFreq));
    velocity *= deltaTime.asSeconds();
    
    sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
    if(!map->checkCollision(nextPosX, eStats.radius)) m_position.x = nextPosX.x;

    sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
    if(!map->checkCollision(nextPosY, eStats.radius)) m_position.y = nextPosY.y;

    return deadPlayers;
}