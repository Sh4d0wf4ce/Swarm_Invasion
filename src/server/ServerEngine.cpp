#include "ServerEngine.hpp"
#include <iostream>

ServerEngine::ServerEngine(): m_isRunning(true), m_tickCounter(0){
    m_timePerTick = sf::seconds(1.0f / 60.0f);

    m_map = std::make_shared<MapGenerator>(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES);
    m_map->generate(1337);

    if(m_socket.bind(Config::SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr<<"[SERVER ERROR] Cant bind to port " << Config::SERVER_PORT << "!\n";
        m_isRunning = false;
    }else{
        std::cout<<"[SERVER] Socket UDP open on port " << Config::SERVER_PORT << ".\n";
    }

    m_socket.setBlocking(false);
}

void ServerEngine::run(){
    std::cout<<"[SERVER] Launching engine (Tickrat: 60)...\n";

    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while(m_isRunning){
        sf::Time elapsedTime = m_clock.restart();
        timeSinceLastUpdate += elapsedTime;

        while(timeSinceLastUpdate > m_timePerTick){
            timeSinceLastUpdate -= m_timePerTick;

            processNetwork();
            update(m_timePerTick);
        }

        sf::sleep(sf::milliseconds(1));
    }
}

void ServerEngine::processNetwork(){
    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while(m_socket.receive(packet, sender, port) == sf::Socket::Status::Done){
        PacketType type;
        if(packet >> type){
            if(type == PacketType::Ping){
                std::string message;
                packet >> message;
                std::cout << "[SERVER] Recieved PING from " << sender.value() << ":" << port
                          << " | Message: " << message << "\n";

                sf::Packet reply;
                reply << PacketType::Pong << "Server here!";
                (void)m_socket.send(reply, sender.value(), port);
            }
            else if(type == PacketType::PlayerPosition){
                std::uint32_t playerId;
                sf::Vector2f position;
                
                if(packet >> playerId >> position && sender.has_value()){
                    auto it = m_clients.find(playerId);
                    if(it != m_clients.end()){
                        it->second.position = position;
                        it->second.lastActivity.restart();
                    }
                }
            }
            else if(type == PacketType::EnemyHit){
                std::uint32_t enemyId;
                if(packet >> enemyId){
                    auto it = m_enemies.find(enemyId);
                    if(it != m_enemies.end()){
                        it->second.hp -= Config::PROJECTILE_DAMAGE;
                        
                        if(it->second.hp <= 0.0f){
                            m_enemies.erase(it);
                            std::cout << "[SERVER] Enemy ID: " << enemyId << " died!\n";
                        }
                    }
                }
            }
            else if(type == PacketType::JoinRequest){
                PlayerClass requestedClass;
                if(sender.has_value() && (packet >> requestedClass)){

                    std::uint32_t newId = m_globalEntityCounter++;
                    sf::Vector2f newPos = sf::Vector2f(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES)*Config::TILE_SIZE / 2.0f;
                    const auto& stats = HeroRegistry::getStats(requestedClass);

                    m_clients.insert_or_assign(newId, ClientInfo{
                        sender.value(), port, newPos, sf::Clock(), stats.maxHp, stats.speed, requestedClass
                    });
                    
                    sf::Packet reply;
                    reply << PacketType::JoinAccept << newId;
                    (void)m_socket.send(reply, sender.value(), port);

                    std::cout << "[SERVER] New player joined the game! Given ID: " << newId << "\n";
                }
            }
            else if(type == PacketType::PlayerShoots){
                std::uint32_t shooterId;
                sf::Vector2f startPos, targetPos;
                if(packet >> shooterId >> startPos >> targetPos){
                    sf::Packet relayPacket;
                    relayPacket << PacketType::PlayerShoots << shooterId << startPos << targetPos;

                    for(const auto& [id, info] : m_clients){
                        if(id == shooterId) continue;
                        (void)m_socket.send(relayPacket, info.ip, info.port);
                    }
                }
            }
        }
    }
}

void ServerEngine::update(sf::Time deltaTime){
    m_tickCounter++;

    //--- REMOVING UNACTIVE PLAYERS ---
    for(auto it = m_clients.begin(); it != m_clients.end();){
        if(it->second.lastActivity.getElapsedTime().asSeconds() > Config::NETWORK_TIMEOUT_SECONDS){
            std::cout << "[SERVER] Player ID: " << it->first << " disconected (Timeout). \n";
            it = m_clients.erase(it);
        }else{
            ++it;
        }
    }

    //--- SPAWNING ENEMIES ---
    if(!m_clients.empty() && m_enemySpawnTimer.getElapsedTime().asSeconds() > Config::ENEMY_SPAWN_RATE){
        m_enemySpawnTimer.restart();

        EnemyInfo newEnemy;
        newEnemy.position = sf::Vector2f(100.0f, 100.0f);
        newEnemy.speed = Config::ENEMY_SPEED;

        std::uint32_t enemyId = m_globalEntityCounter++;
        m_enemies[enemyId] = newEnemy;
        std::cout << "[SERVER] Spawned enemy ID: " << enemyId - 1 << "\n";
    }

    //--- ENEMIES AI ---
    if(!m_clients.empty()){
        auto targetIt = m_clients.begin();
        std::uint32_t targetId = targetIt->first;
        sf::Vector2f targetPos = targetIt->second.position;

        for(auto& [id, enemy] : m_enemies){
            sf::Vector2 direction = targetPos - enemy.position;
            float lenSq = direction.lengthSquared();
            float playerRadius = HeroRegistry::getStats(targetIt->second.pClass).radius;
            float touchDist = Config::ENEMY_RADIUS + playerRadius;
            
            if(lenSq < touchDist * touchDist){
                if(enemy.lastAttackTime.getElapsedTime().asSeconds() > Config::ENEMY_ATTACK_COOLDOWN){
                    targetIt->second.hp -= Config::ENEMY_DAMAGE;
                    enemy.lastAttackTime.restart();
                    std::cout << "[SERVER] Player " << targetId << "got bitten! HP left: " << targetIt->second.hp << "\n";

                    if(targetIt->second.hp <= 0.0f) {
                        std::cout << "[SERVER] Player " << targetId << " died!\n";

                        sf::Packet deathPacket;
                        deathPacket << PacketType::PlayerDied;
                        (void)m_socket.send(deathPacket, targetIt->second.ip, targetIt->second.port);

                        m_clients.erase(targetId);
                        break;
                    }
                }
            }

            
            if(lenSq > 0){
                direction /= std::sqrt(lenSq);

                sf::Vector2f velocity = direction * enemy.speed * deltaTime.asSeconds();

                sf::Vector2f nextPosX = enemy.position + sf::Vector2f(velocity.x, 0.0f);
                if(!checkCollision(nextPosX, Config::ENEMY_RADIUS))
                    enemy.position.x = nextPosX.x;

                sf::Vector2f nextPosY = enemy.position + sf::Vector2f(0.0f, velocity.y);
                if(!checkCollision(nextPosY, Config::ENEMY_RADIUS))
                    enemy.position.y = nextPosY.y;
            }
        }
    }

    if(m_tickCounter % 60 == 0){
        std::cout<<"[SERVER] Server is ticking... Active time: " << (m_tickCounter/60) << "s\n";
    }

    //--- CREATING WORLD STATE PACKET ---
    if(!m_clients.empty()){
        sf::Packet worldPacket;

        // Players info
        worldPacket << PacketType::WorldState << static_cast<std::uint32_t>(m_clients.size());
        for(const auto& [clientId, info] : m_clients){
            worldPacket << clientId << info.pClass << info.position << info.hp;
        }

        // Enemy info
        worldPacket << static_cast<std::uint32_t>(m_enemies.size());
        for(const auto& [enemyId, enemy] : m_enemies){
            worldPacket << enemyId << enemy.position << enemy.hp;
        }

        // Sending
        for(const auto& [clientId, info] : m_clients){
            (void)m_socket.send(worldPacket, info.ip, info.port);
        }
    }
}

bool ServerEngine::checkCollision(const sf::Vector2f& pos, float radius){
    if(!m_map) return false;

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
    
        if(m_map->getTile(gridX, gridY) == TileType::Wall)
            return true;
    }

    return false;
}