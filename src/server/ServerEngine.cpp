#include "ServerEngine.hpp"
#include <iostream>

ServerEngine::ServerEngine(): m_isRunning(true), m_tickCounter(0){
    m_timePerTick = sf::seconds(1.0f / 60.0f);

    if(m_socket.bind(54000) != sf::Socket::Status::Done) {
        std::cerr<<"[SERVER ERROR] Cant bind to port 54000!\n";
        m_isRunning = false;
    }else{
        std::cout<<"[SERVER] Socket UDP open on port 54000.\n";
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
                    if(m_enemies.erase(enemyId) > 0){
                        std::cout << "[SERVER] Enemy ID: " << enemyId << " died!\n";
                    }
                }
            }
            else if(type == PacketType::JoinRequest){
                if(!sender.has_value()) continue;

                std::uint32_t newId = m_nextPlayerId++;
                m_clients.insert_or_assign(newId, ClientInfo{sender.value(), port, sf::Vector2f(640.0f, 360.0f), sf::Clock()});
                
                sf::Packet reply;
                reply << PacketType::JoinAccept << newId;
                (void)m_socket.send(reply, sender.value(), port);

                std::cout << "[SERVER] New player joined the game! Given ID: " << newId << "\n";
            }
        }
    }
}

void ServerEngine::update(sf::Time deltaTime){
    m_tickCounter++;

    //--- REMOVING UNACTIVE PLAYERS ---
    for(auto it = m_clients.begin(); it != m_clients.end();){
        if(it->second.lastActivity.getElapsedTime().asSeconds() > 20.0f){
            std::cout << "[SERVER] Player ID: " << it->first << " disconected (Timeout). \n";
            it = m_clients.erase(it);
        }else{
            ++it;
        }
    }

    //--- SPAWNING ENEMIES ---
    if(!m_clients.empty() && m_enemySpawnTimer.getElapsedTime().asSeconds() > 2.0f){
        m_enemySpawnTimer.restart();

        EnemyInfo newEnemy;
        newEnemy.position = sf::Vector2f(100.0f, 100.0f);
        newEnemy.speed = 100.0f;

        m_enemies[m_nextEnemyId++] = newEnemy;
        std::cout << "[SERVER] Spawned enemy ID: " << m_nextEnemyId - 1 << "\n";
    }

    //--- ENEMIES AI ---
    if(!m_clients.empty()){
        sf::Vector2f targetPos = m_clients.begin()->second.position;

        for(auto& [id, enemy] : m_enemies){
            sf::Vector2 direction = targetPos - enemy.position;
            float length = direction.length();

            if(length > 0){
                direction /= length;
                enemy.position += direction * enemy.speed * deltaTime.asSeconds();
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
        for(const auto& [id, client] : m_clients){
            worldPacket << id << client.position;
        }

        // Enemy info
        worldPacket << static_cast<std::uint32_t>(m_enemies.size());
        for(const auto& [id, enemy] : m_enemies){
            worldPacket << id << enemy.position;
        }

        // Sending
        for(const auto& [id, client] : m_clients){
            (void)m_socket.send(worldPacket, client.ip, client.port);
        }
    }
}