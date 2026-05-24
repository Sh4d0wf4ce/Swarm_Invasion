#include "../enemies/ServerBruiser.hpp"
#include "../enemies/ServerSpitter.hpp"
#include "../enemies/ServerCrawler.hpp"
#include "../enemies/ServerKamikaze.hpp"
#include "AIDirector.hpp"
#include "Config.hpp"
#include "EnemyRegistry.hpp"
#include "HeroRegistry.hpp"
#include <iostream>
#include <cmath>

AIDirector::AIDirector() : m_currentWave(1), m_currentSpawnRate(0.1f){ // 2.0f
    std::random_device rd;
    m_rng.seed(rd());
}

void AIDirector::updateWaves(sf::Time deltaTime, std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& enemies, const std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, std::uint32_t& entityCounter){
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

            std::vector<std::pair<EnemyType, int>> spawnWeights = {
                {EnemyType::Crawler, 1000}, //50
                {EnemyType::Bruiser, 20}, //20
                {EnemyType::Spitter, 20}, //20
                {EnemyType::Kamikaze, 0}, //10
            };

            int totalWeight = 0;
            for(const auto& [type, weight] : spawnWeights){
                totalWeight += weight;
            }

            int rng = rand() % totalWeight;
            EnemyType randomType = EnemyType::Crawler;
            int currentSum = 0;;

            for(const auto& [type, weight] : spawnWeights){
                currentSum += weight;
                if(rng < currentSum){
                    randomType = type;
                    break;
                }
            }

            const auto& stats = EnemyRegistry::getStats(randomType);

            spawnPos = sf::Vector2f((tx + 0.5f) * Config::TILE_SIZE, (ty + 0.5f) * Config::TILE_SIZE);

            if(!map->checkCollision(spawnPos, stats.radius)){   
                bool tooClose = false;
                for(const auto& [id, info] : clients){
                    float distSq = (spawnPos - info.position).lengthSquared();
                    if(distSq < 400.0f * 400.0f){
                        tooClose = true;
                        break;
                    }
                }
    
                if(!tooClose){
                    if (randomType == EnemyType::Crawler) enemies[entityCounter] = std::make_unique<ServerCrawler>(entityCounter, spawnPos, randomType);
                    else if (randomType == EnemyType::Bruiser) enemies[entityCounter] = std::make_unique<ServerBruiser>(entityCounter, spawnPos, randomType);
                    else if (randomType == EnemyType::Kamikaze) enemies[entityCounter] = std::make_unique<ServerKamikaze>(entityCounter, spawnPos, randomType);
                    else enemies[entityCounter] = std::make_unique<ServerSpitter>(entityCounter, spawnPos, randomType);
                    
                    entityCounter++;
                    break;
                }
            }

        }
    }
}


std::vector<std::uint32_t> AIDirector::updateBehaviours(sf::Time deltaTime, std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& enemies, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, std::vector<EnemyShootEvent>& outShootEvents){
    std::vector<std::uint32_t> deadPlayers;
    if(clients.empty()) return deadPlayers;

    m_grid.clear();
    for(const auto& [id, enemy] : enemies){
        m_grid.insert(enemy->getPosition(), id);
    }

    if(m_pathFindingTimer.getElapsedTime().asSeconds() > 0.25f){
        buildFlowField(map, clients);
        m_pathFindingTimer.restart();
    }


    for(auto& [id, enemy] : enemies){
        auto killedByThisEnemy = enemy->update(deltaTime, clients, map, m_flowField, outShootEvents, enemies, m_grid);
        deadPlayers.insert(deadPlayers.end(), killedByThisEnemy.begin(), killedByThisEnemy.end());
    }

    return deadPlayers;
}


void AIDirector::buildFlowField(std::shared_ptr<MapGenerator> map, const std::map<uint32_t, ClientInfo>& clients){
    if(!map || clients.empty()) return;

    int width = map->getWidth();
    int height = map->getHeight();

    if (m_flowField.empty() || m_flowField.size() != width) {
        m_flowField.assign(width, std::vector<int>(height, MAXINT32));
    } else {
        for (int x = 0; x < width; ++x) {
            std::fill(m_flowField[x].begin(), m_flowField[x].end(), MAXINT32);
        }
    }
    
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