#include "ClientEngine.hpp"
#include <iostream>


ClientEngine::ClientEngine(): m_isRunning(true){
    m_window.create(sf::VideoMode({1280, 720}), "Swarm Invasion - Client", sf::Style::Default);
    m_window.setFramerateLimit(60);

    if(!ImGui::SFML::Init(m_window)){
        std::cerr<<"Error: Failed to initialize ImGui\n";
    }

    m_socket.setBlocking(false);

    std:srand(std::time(nullptr));
    std::uint32_t id = std::rand() % 1000;

    m_player = std::make_unique<Player>(id, sf::Vector2f(640.0f, 360.0f));
    m_lastSentPosition = sf::Vector2f(-9999.0f, -9999.0f);

    m_serverAddress = sf::IpAddress::resolve("127.0.0.1");

    m_map = std::make_shared<MapGenerator>(40, 22);
    m_map->generate(1337);
    m_mapRenderer = std::make_unique<MapRenderer>(m_map, 32.0f);

    m_projectileManager = std::make_unique<ProjectileManager>();
}

void ClientEngine::run(){
    while(m_isRunning && m_window.isOpen()){
        sf::Time deltaTime = m_clock.restart();

        processEvent();
        processNetwork();
        update(deltaTime);
        render();
    }

    ImGui::SFML::Shutdown();
}

void ClientEngine::processEvent(){
    while(const std::optional event = m_window.pollEvent()){
        ImGui::SFML::ProcessEvent(m_window, *event);

        if(event->is<sf::Event::Closed>()){
            m_isRunning = false;
            m_window.close();
        }

        if(const auto& mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()){
            sf::Vector2f targetPos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
            m_projectileManager->spawnProjectile(m_player->getPosition(), targetPos, 800.0f);
        }
    }
}

void ClientEngine::processNetwork(){
    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while(m_socket.receive(packet, sender, port) == sf::Socket::Status::Done){
        PacketType type;
        if(packet >> type){
            if(type == PacketType::Pong){
                std::string message;
                packet >> message;
                std::cout<< "[CLIENT] Recieved PONG from server: " << message << std::endl;
            }
            else if(type == PacketType::WorldState){
                std::uint32_t playerCount;
                if(packet >> playerCount){

                    std::vector<std::uint32_t> activeServerIds;

                    for(std::uint32_t i = 0; i < playerCount; i++){
                        std::uint32_t id;
                        sf::Vector2f pos;
                        packet >> id >> pos;

                        activeServerIds.push_back(id);

                        if(id == m_player->getId()) continue;

                        if(m_otherPlayers.find(id) == m_otherPlayers.end()){
                            m_otherPlayers[id] = std::make_unique<Player>(id, pos);
                            m_otherPlayers[id]->setColor(sf::Color::Blue);
                        }

                        m_otherPlayers[id]->setPosition(pos);
                    }

                    for(auto it = m_otherPlayers.begin(); it != m_otherPlayers.end();){
                        if(std::find(activeServerIds.begin(), activeServerIds.end(), it->first) == activeServerIds.end()){
                            it = m_otherPlayers.erase(it);
                        }else{
                            ++it;
                        }
                    }
                }
            }
        }
    }
}

void ClientEngine::update(sf::Time deltaTime){
    ImGui::SFML::Update(m_window, deltaTime);

    if(m_player){
        m_player->setFocused(m_window.hasFocus());
        m_player->update(deltaTime);

        if(m_player->getPosition() != m_lastSentPosition){
            sf::Packet packet;
            packet << PacketType::PlayerPosition << m_player->getId() << m_player->getPosition();

            if(m_serverAddress){
                (void)m_socket.send(packet, m_serverAddress.value(), 54000);
            }

            m_lastSentPosition = m_player->getPosition();
        }
    }

    if(m_projectileManager){
        m_projectileManager->update(deltaTime);
    }
}

void ClientEngine::render(){
    m_window.clear(sf::Color(30, 30, 30));

    if(m_mapRenderer) m_mapRenderer->render(m_window);
    if(m_projectileManager) m_projectileManager->render(m_window);
    if(m_player) m_player->render(m_window);

    for(const auto& [id, otherPlayer]: m_otherPlayers){
        otherPlayer->render(m_window);
    }

    renderUI();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void ClientEngine::renderUI(){
    ImGui::Begin("Swarm Inasion - Debug Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if(ImGui::Button("Send PING to server!")){
        sf::Packet packet;

        if(m_serverAddress){
            packet << PacketType::Ping << "Hello Server, are you there?";
            if(m_socket.send(packet, m_serverAddress.value(), 54000) != sf::Socket::Status::Done){
                std::cout << "[CLIENT] Error sending PING...\n";
            }else{
                std::cout << "[CLIENT] Sent PING...\n";
            }
        }else{
            std::cerr<<"[CLIENT] IP resolve error!\n";
        }
    }

    if(ImGui::Button("Close game")){
        m_isRunning = false;
    }



    if(ImGui::Button("Draw random seed")){
        m_map->generate(rand() % 10000);
        m_mapRenderer = std::make_unique<MapRenderer>(m_map, 32.0f);
    }

    ImGui::End();
}