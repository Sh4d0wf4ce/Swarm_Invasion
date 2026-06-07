#include "ServerBruiser.hpp"
#include "HeroRegistry.hpp"

std::vector<std::uint32_t> ServerBruiser::update(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, std::vector<EnemyShootEvent>& outShootEvents, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid) {
    std::vector<std::uint32_t> deadPlayers;
    if(clients.empty()) return deadPlayers;

    const auto& eStats = EnemyRegistry::getStats(m_type);
    float minDistanceSq;
    std::uint32_t targetId = getClosestPlayerId(clients, minDistanceSq);
    float chargeRange = 300.0f;
    
    switch(m_state){
        case BruiserState::Chasing:
            // --- Transition to charge when in range with line of sight ---
            if(m_cooldownTimer.getElapsedTime().asSeconds() > 5.0f && minDistanceSq < (chargeRange * chargeRange)){
                if(hasLineOfSight(m_position, clients.at(targetId).position, eStats.radius, map)){
                    m_state = BruiserState::Preparing;
                    m_stateTimer.restart();
                    sf::Vector2f dir = (clients.at(targetId).position - m_position);
                    float len = dir.length();
                    if(len > 0.0f) m_chargeDirection = dir / len;
                }
            } else{
                return performMeleeChase(deltaTime, clients, map, flowField, allEnemies, grid);
            }
            break;
        case BruiserState::Preparing:
            // --- Brief wind-up before charge ---
            if(m_stateTimer.getElapsedTime().asSeconds() > 0.2f){
                m_state = BruiserState::Charging;
                m_stateTimer.restart();
            }
            break;
        case BruiserState::Charging: {
            // --- Rush forward and deal contact damage ---
            float chargeSpeed = m_speed * 8.0f;
            sf::Vector2f velocity = m_chargeDirection * chargeSpeed * deltaTime.asSeconds();
            sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
            bool hitWallX = map->checkCollision(nextPosX, eStats.radius);
            if(!hitWallX) m_position.x = nextPosX.x;
            sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
            bool hitWallY = map->checkCollision(nextPosY, eStats.radius);
            if(!hitWallY) m_position.y = nextPosY.y;
            for(auto& [playerId, playerInfo] : clients){
                float distSq = (playerInfo.position - m_position).lengthSquared();
                float touchDist = eStats.radius + HeroRegistry::getStats(playerInfo.pClass).radius;
                if(distSq < touchDist * touchDist){
                    if(m_lastAttackTime.getElapsedTime().asSeconds() > eStats.attackCooldown){
                        playerInfo.hp -= eStats.damage * 1.5f;
                        m_lastAttackTime.restart();
                        if(playerInfo.hp <= 0.0f) deadPlayers.push_back(playerId);
                    }
                }
            }
            if(hitWallX || hitWallY || m_stateTimer.getElapsedTime().asSeconds() > 1.0f){
                m_state = BruiserState::Resting;
                m_stateTimer.restart();
            }
            break;
        }
        case BruiserState::Resting:
            // --- Recover before returning to chase ---
            if(m_stateTimer.getElapsedTime().asSeconds() > 1.0f) {
                m_state = BruiserState::Chasing;
                m_cooldownTimer.restart();
            }
            break;
    }
    return deadPlayers;
}
