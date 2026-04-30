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
                if(m_socket.send(reply, sender.value(), port) == sf::Socket::Status::Done);
            }
        }
    }
}

void ServerEngine::update(sf::Time deltaTime){
    m_tickCounter++;

    if(m_tickCounter % 60 == 0){
        std::cout<<"[SERVER] Server is ticking... Active time: " << (m_tickCounter/60) << "s\n";
    }
}