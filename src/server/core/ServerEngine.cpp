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
        if(!sender.has_value()) continue;

        PacketType type;
        if(packet >> type){
            switch(type){
                case PacketType::Ping:              handlePing(packet, sender.value(), port); break;
                case PacketType::PlayerPosition:    handlePlayerPosition(packet); break;
                case PacketType::EntityHit:         handleEntityHit(packet); break;
                case PacketType::JoinRequest:       handleJoinRequest(packet, sender.value(), port); break;
                case PacketType::PlayerShoots:      handlePlayerShoots(packet); break;
                case PacketType::PlayerDisconnect:  handlePlayerDisconnect(packet); break;
                case PacketType::CardSelected:      handleCardSelected(packet); break;
                default: break;
            }
        }
    }
}

void ServerEngine::update(sf::Time deltaTime){
    m_tickCounter++;

    // CARD SELECTION SCREEN
    if(m_isPaused){
        proccessUpgradeMenuTimeout();
        sendWorldState();
        return;
    }

    // REMOVING UNACTIVE PLAYERS
    removeAFKPlayers();


    // SERVER RESET
    if(m_clients.empty() && !m_enemies.empty()){
        m_enemies.clear();
        std::cout << "[SERVER] All players left. Resetting world...\n";
    }

    // ENEMIES DIRECTOR
    if(!m_clients.empty()){
        m_aiDirector.updateWaves(deltaTime, m_enemies, m_clients, m_map, m_globalEntityCounter);
        
        std::vector<EnemyShootEvent> shootEvents;
        auto deadPlayers = m_aiDirector.updateBehaviours(deltaTime, m_enemies, m_clients, m_map, shootEvents);

        for(std::uint32_t deadId : deadPlayers){
            if(m_clients.count(deadId)){
                auto& targetInfo = m_clients.at(deadId);
                std::cout << "[SERVER] Player " << deadId << " died!\n";

                sf::Packet deathPacket;
                deathPacket << PacketType::PlayerDied;
                (void)m_socket.send(deathPacket, targetInfo.ip, targetInfo.port);

                m_clients.erase(deadId);
            }
        }

        for(const auto& shoot: shootEvents){
            sf::Packet shootPacket;
            shootPacket << PacketType::EnemyShoots << shoot.weapon << shoot.startPos << shoot.targetPos;

            for(const auto& [playerId, info] : m_clients){
                (void)m_socket.send(shootPacket, info.ip, info.port);
            }
        }
    }

    for(auto it = m_enemies.begin(); it != m_enemies.end(); ){
        if(it->second->getHp() <= 0.0f) it = m_enemies.erase(it);
        else ++it;
    }

    // EXP SYSTEM
    updateEnergyCells(deltaTime);


    // SENDING CURRENT WORLD STATE TO CLiENTS
    if(m_tickCounter % 60 == 0){
            std::cout<<"[SERVER] Server is ticking... Active time: " << (m_tickCounter/60) << "s\n";
    }
    sendWorldState();
}

void ServerEngine::handlePing(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port){
    std::string message;
    packet >> message;
    std::cout << "[SERVER] Recieved PING from " << sender << ":" << port << " | Message: " << message << "\n";
    sf::Packet reply;
    reply << PacketType::Pong << "Server here!";
    (void)m_socket.send(reply, sender, port);
}

void ServerEngine::handlePlayerPosition(sf::Packet& packet){
    std::uint32_t playerId;
    sf::Vector2f position;

    if(packet >> playerId >> position){
        auto it = m_clients.find(playerId);
        if(it != m_clients.end()){
            it->second.position = position;
            it->second.lastActivity.restart();
        }
    }
}

void ServerEngine::handleEntityHit(sf::Packet& packet){
    std::uint32_t targetId;
    WeaponType weaponUsed;

    if(packet >> targetId >> weaponUsed){
        float damage = WeaponRegistry::getStats(weaponUsed).damage;

        // Player hit
        if(m_clients.count(targetId)){
            m_clients.at(targetId).hp -= damage;

            if(m_clients.at(targetId).hp <= 0.0f){
                sf::Packet deathPacket;
                deathPacket << PacketType::PlayerDied;
                (void)m_socket.send(deathPacket, m_clients.at(targetId).ip, m_clients.at(targetId).port);
            }
        }
        // Enemy hit
        else if(m_enemies.count(targetId)){
            m_enemies[targetId]->takeDamage(damage);

            if(m_enemies[targetId]->getHp() <= 0.0f){
                m_energyCells[m_globalEntityCounter++] = {m_enemies[targetId]->getPosition(), 1, 0};
                m_enemies.erase(targetId);
            }
        }
    }
}

void ServerEngine::handleJoinRequest(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port){
    PlayerClass requestedClass;
    if(packet >> requestedClass){
        std::uint32_t newId = m_globalEntityCounter++;
        sf::Vector2f newPos = sf::Vector2f(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES)*Config::TILE_SIZE / 2.0f;
        const auto& stats = HeroRegistry::getStats(requestedClass);

        m_clients.insert_or_assign(newId, ClientInfo{
            sender, port, newPos, sf::Clock(), stats.maxHp, stats.speed, requestedClass
        });
        
        sf::Packet reply;
        reply << PacketType::JoinAccept << newId;
        (void)m_socket.send(reply, sender, port);
        std::cout << "[SERVER] New player joined the game! Given ID: " << newId << "\n";
    }
}

void ServerEngine::handlePlayerShoots(sf::Packet& packet){
    std::uint32_t shooterId;
    sf::Vector2f startPos, targetPos;
    WeaponType weaponUsed;
    if(packet >> shooterId >> weaponUsed >> startPos >> targetPos){
        sf::Packet relayPacket;
        relayPacket << PacketType::PlayerShoots << shooterId << weaponUsed << startPos << targetPos;

        for(const auto& [id, info] : m_clients){
            if(id == shooterId) continue;
            (void)m_socket.send(relayPacket, info.ip, info.port);
        }
    }
}

void ServerEngine::handlePlayerDisconnect(sf::Packet& packet){
    std::uint32_t playerId;
    if(packet >> playerId){
        m_clients.erase(playerId);
        std::cout << "[SERVER] Player " << playerId << " disconnected\n";
    }
}

void ServerEngine::handleCardSelected(sf::Packet& packet){
    std::uint32_t playerId;
    int choice;
    if(packet >> playerId >> choice){
        m_playerChoices[playerId] = choice;
    }
}


void ServerEngine::proccessUpgradeMenuTimeout(){
    bool allSelected = true;
    for(auto& [id, info] : m_clients){
        if(m_playerChoices[id] == -1) allSelected = false;
    }

    if(allSelected || m_upgradeTimer.getElapsedTime().asSeconds() > Config::LEVEL_UP_TIMEOUT){
        m_isPaused = false;
        std::cout << "[SERVER] Resuming game after upgrades. \n";
    }
}

void ServerEngine::removeAFKPlayers(){
    for(auto it = m_clients.begin(); it != m_clients.end();){
        if(it->second.lastActivity.getElapsedTime().asSeconds() > Config::NETWORK_TIMEOUT_SECONDS){
            std::cout << "[SERVER] Player ID: " << it->first << " disconected (Timeout). \n";
            it = m_clients.erase(it);
        }else{
            ++it;
        }
    }
}

void ServerEngine::updateEnergyCells(sf::Time deltaTime){
    for(auto it = m_energyCells.begin(); it != m_energyCells.end(); ){
        auto& cell = it->second;

        // 1. looking for closest player;
        if(cell.targetPlayerId == 0){
            float closestDist = std::pow(Config::MAGNET_RADIUS, 2);
            for(const auto& [pId, pInfo] : m_clients){
                float distSq = (cell.position - pInfo.position).lengthSquared();
                if(distSq < closestDist){
                    closestDist = distSq;
                    cell.targetPlayerId = pId;
                }
            }
        }

        // 2. exp goes to the closest player in magnet radius
        if(cell.targetPlayerId != 0 && m_clients.count(cell.targetPlayerId)){
            auto& pInfo = m_clients.at(cell.targetPlayerId);
            sf::Vector2f dir = pInfo.position - cell.position;
            float distSq = dir.lengthSquared();

            if(distSq < std::pow(Config::PICKUP_RADIUS, 2)){
                m_teamExp += cell.expValue;
                if(m_teamExp >= m_teamExpMax){
                    m_teamExp -= m_teamExpMax;
                    m_teamLevel++;
                    m_teamExpMax = static_cast<int>(m_teamExpMax * 1.5f);
                    std::cout << "[SERVER] Players reached level " << m_teamLevel << "!\n";

                    m_isPaused = true;
                    m_upgradeTimer.restart();
                    m_playerChoices.clear();
                    for(auto& [id, info] : m_clients) m_playerChoices[id] = -1;

                    sf::Packet pausePacket;
                    pausePacket << PacketType::LevelUpTriggered;
                    for(auto& [id, info] : m_clients){
                        (void)m_socket.send(pausePacket, info.ip, info.port);
                    }
                }
                it = m_energyCells.erase(it);
                continue;
            }else if(distSq > 0){
                cell.position += (dir / std::sqrt(distSq)) * Config::CRYSTAL_SPEED * deltaTime.asSeconds();
            }
        }else{
            cell.targetPlayerId = 0;
        }
        ++it;
    }
}

void ServerEngine::sendWorldState(){
    if(m_clients.empty()) return;

    sf::Packet worldPacket;

    // Players info
    worldPacket << PacketType::WorldState << static_cast<std::uint32_t>(m_clients.size());
    
    for(const auto& [clientId, info] : m_clients){
        worldPacket << clientId << info.pClass << info.position << info.hp;
    }

    // Exp info
    worldPacket << m_teamLevel << m_teamExp << m_teamExpMax << m_isPaused;

    // Enemy info
    worldPacket << static_cast<std::uint32_t>(m_enemies.size());
    for(const auto& [enemyId, enemy] : m_enemies){
        worldPacket << enemyId << enemy->getType() << enemy->getPosition() << enemy->getHp();
    }

    // Energy Cells info
    worldPacket << static_cast<std::uint32_t>(m_energyCells.size());
    for(const auto& [cellId, cell] : m_energyCells){
        worldPacket << cellId << cell.position;
    }

    // Sending
    for(const auto& [clientId, info] : m_clients){
        (void)m_socket.send(worldPacket, info.ip, info.port);
    }
}
