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

            EnemyType randomType = (rand() % 100 < 70) ? EnemyType::Crawler : EnemyType::Bruiser;
            const auto& stats = EnemyRegistry::getStats(randomType);

            spawnPos = sf::Vector2f((tx + 0.5f) * Config::TILE_SIZE, (ty + 0.5f) * Config::TILE_SIZE);

            if(!checkCollision(spawnPos, stats.radius, map)){   
                bool tooClose = false;
                for(const auto& [id, info] : clients){
                    float distSq = (spawnPos - info.position).lengthSquared();
                    if(distSq < 400.0f * 400.0f){
                        tooClose = true;
                        break;
                    }
                }
    
                if(!tooClose){
                    enemies[entityCounter++] = {spawnPos, stats.speed, stats.maxHp, sf::Clock(), randomType};
                    break;
                }
            }

        }
    }
}


std::vector<std::uint32_t> AIDirector::updateBehaviours(sf::Time deltaTime, std::map<std::uint32_t, EnemyInfo>& enemies, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map){
    std::vector<std::uint32_t> deadPlayers;
    if(clients.empty()) return deadPlayers;

    if(m_pathFindingTimer.getElapsedTime().asSeconds() > 0.25f){
        buildFlowField(map, clients);
        m_pathFindingTimer.restart();
    }


    for(auto& [id, enemy] : enemies){
        const auto& eStats = EnemyRegistry::getStats(enemy.type);

        std::uint32_t targetId = clients.begin()->first;
        float minDistanceSq  = (clients.begin()->second.position - enemy.position).lengthSquared();

        // DEALING DAMAGE TO PLAYERS
        for(auto& [playerId, playerInfo] : clients){
            float distSq = (playerInfo.position - enemy.position).lengthSquared();

            if(distSq < minDistanceSq){
                minDistanceSq = distSq;
                targetId = playerId;
            }

            float touchDist = eStats.radius + HeroRegistry::getStats(playerInfo.pClass).radius;

            if(distSq < touchDist * touchDist){
                if(enemy.lastAttackTime.getElapsedTime().asSeconds() > eStats.attackCooldown){
                    playerInfo.hp -= eStats.damage;
                    enemy.lastAttackTime.restart();
                    std::cout << "[AI DIRECTOR] Player " << playerId << "got bitten! HP left: " << playerInfo.hp << "\n";

                    if(playerInfo.hp <= 0.0f) deadPlayers.push_back(playerId);
                }
            }   
        }

        
        // MOVING ENEMIES
        sf::Vector2f desiredDirection(0.0f, 0.0f);

        if(hasLineOfSight(enemy.position, clients.at(targetId).position, eStats.radius, map)){
            desiredDirection = clients.at(targetId).position - enemy.position;
        }
        else if(!m_flowField.empty()){
            int ex = static_cast<int>(enemy.position.x / Config::TILE_SIZE);
            int ey = static_cast<int>(enemy.position.y / Config::TILE_SIZE);
    
            if(ex > 0 && ex < map->getWidth() - 1 && ey > 0 && ey < map->getHeight() - 1){
                int minCost = m_flowField[ex][ey];
                sf::Vector2f bestTarget = enemy.position;
    
                for(int dx = -1; dx <= 1; dx++){
                    for(int dy = -1; dy <= 1; dy++){
                        if(dx == 0 && dy == 0) continue;
    
                        int nx = ex + dx;
                        int ny = ey + dy;
    
                        if(m_flowField[nx][ny] < minCost){
                            minCost = m_flowField[nx][ny];
                            bestTarget = sf::Vector2f((nx + 0.5f) * Config::TILE_SIZE, (ny + 0.5f) * Config::TILE_SIZE);
                        }
                    }
                }
                desiredDirection = bestTarget - enemy.position;
            }
        }


        sf::Vector2f separationVector(0.0f, 0.0f);
        for(const auto& [otherId, otherEnemy]: enemies){
            if(id == otherId) continue;

            sf::Vector2f diff = enemy.position - otherEnemy.position;
            float distSq = diff.lengthSquared();
            float combinedRadius = eStats.radius + EnemyRegistry::getStats(otherEnemy.type).radius;

            if(distSq > 0 && distSq < combinedRadius * combinedRadius){
                separationVector += diff / std::sqrt(distSq);
            }
        }

        if(desiredDirection.lengthSquared() > 0) desiredDirection /= std::sqrt(desiredDirection.lengthSquared());

        sf::Vector2f finalDirection = desiredDirection + (separationVector * 1.5f);
        float lenSq = finalDirection.lengthSquared();

        if(lenSq > 0){
            finalDirection /= std::sqrt(lenSq);
            sf::Vector2f velocity = finalDirection * enemy.speed * deltaTime.asSeconds();

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

void AIDirector::buildFlowField(std::shared_ptr<MapGenerator> map, const std::map<uint32_t, ClientInfo>& clients){
    if(!map || clients.empty()) return;

    int width = map->getWidth();
    int height = map->getHeight();

    m_flowField.assign(width, std::vector<int>(height, MAXINT32));
    
    std::queue<sf::Vector2i> q;

    for(const auto& [id, info]: clients){
        int px = static_cast<int>(info.position.x / Config::TILE_SIZE);
        int py = static_cast<int>(info.position.y / Config::TILE_SIZE);

        if(px > 0 && px < width && py > 0 && py < height){
            m_flowField[px][py] = 0;
            q.push({px, py});
        }
    }

    //directions
    int dx[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    int dy[] = {0, 0, -1, 1, -1, -1, 1, 1};

    //BFS
    while(!q.empty()){
        auto curr = q.front();
        q.pop();

        int currentDist = m_flowField[curr.x][curr.y];

        for(int i = 0; i < 8; i++){
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if(nx <= 0 || nx >= width - 1 || ny <= 0 || ny >= height -1) continue;
            if(map->getTile(nx, ny) == TileType::Wall) continue;

            // cost in not diagonal direction: 10, cost in diagonal direction 10 * sqrt(2) = approx 14
            int moveCost = (i < 4) ? 10 : 14;

            if(currentDist + moveCost < m_flowField[nx][ny]){
                m_flowField[nx][ny] = currentDist + moveCost;
                q.push({nx, ny});
            }
        }
    }
}

bool AIDirector::hasLineOfSight(sf::Vector2f start, sf::Vector2f end, float radius, std::shared_ptr<MapGenerator> map){
    sf::Vector2f dir = end - start;
    float dist = dir.length();
    if(dist == 0) return true;
    dir /= dist;

    float step = Config::TILE_SIZE / 2.0;
    for(float d = 0; d < dist; d += step){
        if(checkCollision(start + dir * d, radius , map)) return false;
    }
    return true;
}