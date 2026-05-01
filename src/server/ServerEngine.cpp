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
                    m_clients.insert_or_assign(playerId, ClientInfo{sender.value(), port, position, sf::Clock()});
                }
            }
        }
    }
}

void ServerEngine::update(sf::Time deltaTime){
    m_tickCounter++;

    for(auto it = m_clients.begin(); it != m_clients.end();){
        if(it->second.lastActivity.getElapsedTime().asSeconds() > 20.0f){
            std::cout << "[SERVER] Player ID: " << it->first << " disconected (Timeout). \n";
            it = m_clients.erase(it);
        }else{
            ++it;
        }
    }

    if(m_tickCounter % 60 == 0){
        std::cout<<"[SERVER] Server is ticking... Active time: " << (m_tickCounter/60) << "s\n";
    }

    if(!m_clients.empty()){
        sf::Packet worldPacket;
        worldPacket << PacketType::WorldState << static_cast<std::uint32_t>(m_clients.size());

        for(const auto& [id, info] : m_clients){
            worldPacket << id << info.position;
        }

        for(const auto& [id, info] : m_clients){
            (void)m_socket.send(worldPacket, info.ip, info.port);
        }
    }
}