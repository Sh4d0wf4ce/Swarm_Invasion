#include "AIDirector.hpp"
#include "Config.hpp"
#include "EnemyRegistry.hpp"
#include "HeroRegistry.hpp"
#include <iostream>
#include <cmath>

AIDirector::AIDirector() : m_currentWave(1), m_currentSpawnRate(2.0f){
    std::random_device rd;
    m_rng.seed(rd());
}

void AIDirector::updateWaves(sf::Time deltaTime, std::map<std::uint32_t, EnemyInfo>& enemies, const std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, std::uint32_t& entityCounter){
    if(m_waveTimer.getElapsedTime().asSeconds() >= 30.0f){
        m_currentWave++;
        m_waveTimer.restart();

        m_currentSpawnRate = std::max(0.1f, m_currentSpawnRate * 0.75f);
        std::cout << "[AI DIRECTOR] WAVE " << m_currentWave << " STARTED! Spawn rate: " << m_currentSpawnRate << "s\n";
    }

    if(m_spawnTimer.getElapsedTime().asSeconds() > m_currentSpawnRate && !clients.empty()){
        m_spawnTimer.restart();

        std::uniform_int_distribution<int> distX(1, map->getWidth() - 2);
        std::uniform_int_distribution<int> distY(1, map->getHeight() - 2);

        sf::Vector2f spawnPos;
        bool validSpawn = false;

        for(int i = 0; i < 50; i++){
            int tx = distX(m_rng);
            int ty = distY(m_rng);

            if(map->getTile(tx, ty) == TileType::Floor){
                spawnPos = sf::Vector2f(tx * Config::TILE_SIZE, ty * Config::TILE_SIZE);

                bool tooClose = false;
                for(const auto& [id, info] : clients){
                    float distSq = (spawnPos - info.position).lengthSquared();
                    if(distSq < 400.0f * 400.0f){
                        tooClose = true;
                        break;
                    }
                }

                if(!tooClose){
                    validSpawn = true;
                    break;
                }
            }
        }

        if(validSpawn){
            EnemyType randomType = (rand() % 100 < 70) ? EnemyType::Crawler : EnemyType::Bruiser;
            const auto& stats = EnemyRegistry::getStats(randomType);
            enemies[entityCounter++] = {spawnPos, stats.speed, stats.maxHp, sf::Clock(), randomType};
        }
    }
}


std::vector<std::uint32_t> AIDirector::updateBehaviours(sf::Time deltaTime, std::map<std::uint32_t, EnemyInfo>& enemies, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map){
    std::vector<std::uint32_t> deadPlayers;
    
    if(clients.empty()) return deadPlayers;


    for(auto& [id, enemy] : enemies){
        std::uint32_t targetId = clients.begin()->first;
        float minDistanceSq = (clients.begin()->second.position - enemy.position).lengthSquared();

        for(const auto& [playerId, playerInfo] : clients){
            float distSq = (playerInfo.position - enemy.position).lengthSquared();
            if(distSq < minDistanceSq){
                minDistanceSq = distSq;
                targetId = playerId;
            }    
        }



        const auto& eStats = EnemyRegistry::getStats(enemy.type);
        ClientInfo& targetInfo = clients.at(targetId);

        sf::Vector2 direction = targetInfo.position - enemy.position;
        float lenSq = direction.lengthSquared();
        float touchDist = eStats.radius + HeroRegistry::getStats(targetInfo.pClass).radius;
        
        if(lenSq < touchDist * touchDist){
            if(enemy.lastAttackTime.getElapsedTime().asSeconds() > eStats.attackCooldown){
                targetInfo.hp -= eStats.damage;
                enemy.lastAttackTime.restart();
                std::cout << "[AI DIRECTOR] Player " << targetId << "got bitten! HP left: " << targetInfo.hp << "\n";

                if(targetInfo.hp <= 0.0f) {
                    deadPlayers.push_back(targetId);
                }
            }
        }

        
        if(lenSq > 0){
            direction /= std::sqrt(lenSq);
            sf::Vector2f velocity = direction * enemy.speed * deltaTime.asSeconds();

            sf::Vector2f nextPosX = enemy.position + sf::Vector2f(velocity.x, 0.0f);
            if(!checkCollision(nextPosX, eStats.radius, map)) enemy.position.x = nextPosX.x;

            sf::Vector2f nextPosY = enemy.position + sf::Vector2f(0.0f, velocity.y);
            if(!checkCollision(nextPosY, eStats.radius, map)) enemy.position.y = nextPosY.y;
        }
    }

    return deadPlayers;
}

bool AIDirector::checkCollision(const sf::Vector2f& pos, float radius, std::shared_ptr<MapGenerator> map){
    if(!map) return false;

    float hitBoxOffset = radius * 0.8f;

    sf::Vector2f points[4] = {
        {pos.x - hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x - hitBoxOffset, pos.y + hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y + hitBoxOffset}
    };

    for(const auto& p: points){
        int gridX = static_cast<int>(p.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(p.y / Config::TILE_SIZE);
    
        if(map->getTile(gridX, gridY) == TileType::Wall) return true;
    }

    return false;
}