#include "ClientEngine.hpp"
#include <iostream>


ClientEngine::ClientEngine(): m_isRunning(true){
    m_window.create(sf::VideoMode({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT}), "Swarm Invasion - Client", sf::Style::Default);
    m_window.setFramerateLimit(60);

    if(!ImGui::SFML::Init(m_window)){
        std::cerr<<"Error: Failed to initialize ImGui\n";
    }

    m_camera.setSize({static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)});

    m_socket.setBlocking(false);
    m_serverAddress = sf::IpAddress::resolve("127.0.0.1");

    m_map = std::make_shared<MapGenerator>(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES);
    m_map->generate(1337);
    m_mapRenderer = std::make_unique<MapRenderer>(m_map, Config::TILE_SIZE);

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
            if(m_player){
                sf::Vector2i pixelPos(mouseBtn->position.x, mouseBtn->position.y);
                sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera); 

                WeaponType myWeapon = HeroRegistry::getStats(m_player->getClass()).defaultWeapon;
                m_projectileManager->spawnProjectile(m_player->getId(), m_player->getPosition(), worldPos, myWeapon);

                if(m_serverAddress){
                    sf::Packet shootPacket;
                    shootPacket << PacketType::PlayerShoots << m_player->getId() << myWeapon << m_player->getPosition() << worldPos;
                    (void)m_socket.send(shootPacket, m_serverAddress.value(), Config::SERVER_PORT);
                }
            }
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
            m_lastServerMessageTimer.restart();

            if(type == PacketType::Pong){
                std::string message;
                packet >> message;
                std::cout<< "[CLIENT] Recieved PONG from server: " << message << std::endl;
            }
            else if(type == PacketType::WorldState){
                handleWorldState(packet);
            }
            else if(type == PacketType::JoinAccept){
                std::uint32_t playerId;
                if(packet >> playerId){
                    if(!m_player){
                        sf::Vector2f newPos = sf::Vector2f(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES) * Config::TILE_SIZE / 2.0f;
                        m_player = std::make_unique<Player>(playerId, newPos, m_selectedClass);
                        m_lastSentPosition = sf::Vector2f(INFINITY, INFINITY);

                        std::cout << "[CLIENT] Player joined the server. Player ID: " << playerId << "\n";
                    }
                }
            }
            else if(type == PacketType::PlayerShoots){
                std::uint32_t shooterId;
                WeaponType weaponUsed;
                sf::Vector2f startPos, targetPos;
                if(packet >> shooterId >> weaponUsed >> startPos >> targetPos){
                    if(!m_projectileManager) continue;
                    m_projectileManager->spawnProjectile(shooterId, startPos, targetPos, weaponUsed);
                }
            }
            else if(type == PacketType::PlayerDied){
                std::cout<< "[CLIENT] You died! Leaving the game... \n";
                m_player.reset();
                m_otherPlayers.clear();
                m_enemies.clear();
                return;
            }
        }
    }
}

void ClientEngine::handleWorldState(sf::Packet& packet){
    std::uint32_t playerCount;
    if(!(packet >> playerCount)) return;

    // --- PLAYERS ---
    std::vector<std::uint32_t> activeServerIds;
    for(std::uint32_t i = 0; i < playerCount; i++){
        std::uint32_t id;
        PlayerClass pClass;
        sf::Vector2f pos;
        float hp;
        packet >> id >> pClass >> pos >> hp;

        activeServerIds.push_back(id);

        if(m_player && id == m_player->getId()){
            m_player->setHp(hp);   
            continue;
        }

        if(m_otherPlayers.find(id) == m_otherPlayers.end()){
            m_otherPlayers[id] = std::make_unique<Player>(id, pos, pClass);
        }

        m_otherPlayers[id]->setHp(hp);
        m_otherPlayers[id]->setPosition(pos);
    }

    // removing unactive players
    for(auto it = m_otherPlayers.begin(); it != m_otherPlayers.end();){
        if(std::find(activeServerIds.begin(), activeServerIds.end(), it->first) == activeServerIds.end()){
            it = m_otherPlayers.erase(it);
        }else{
            ++it;
        }
    }

    // --- ENEMIES ---
    std::uint32_t enemyCount;
    if(!(packet >> enemyCount)) return;
    
    std::vector<std::uint32_t> activeEnemyIds;

    for(std::uint32_t i = 0; i < enemyCount; i++){
        std::uint32_t id;
        sf::Vector2f pos;
        EnemyType eType;
        float hp;
        packet >> id >> eType >> pos >> hp;

        activeEnemyIds.push_back(id);

        if(m_enemies.find(id) == m_enemies.end()){
            m_enemies[id] = std::make_unique<Enemy>(id, pos, eType);
        }
        m_enemies[id]->setPosition(pos);
        m_enemies[id]->setHp(hp);
    }

    for(auto it = m_enemies.begin(); it != m_enemies.end();){
        if(std::find(activeEnemyIds.begin(), activeEnemyIds.end(), it->first) == activeEnemyIds.end()){
            it = m_enemies.erase(it);
        }else{
            ++it;
        }
    }   
}

void ClientEngine::update(sf::Time deltaTime){
    ImGui::SFML::Update(m_window, deltaTime);

    if(m_player){
        if(m_lastServerMessageTimer.getElapsedTime().asSeconds() > Config::NETWORK_TIMEOUT_SECONDS){
            std::cout << "[CLIENT] Lost connection to the server (Timeout)!\n";

            m_player.reset();
            m_otherPlayers.clear();
            m_enemies.clear();

            return;
        }

        m_player->setFocused(m_window.hasFocus());
        m_player->update(deltaTime, m_map);


        sf::Vector2f targetCenter = m_player->getPosition();

        if(m_map){
            float mapWidth = m_map->getWidth() * Config::TILE_SIZE;
            float mapHeight = m_map->getHeight() * Config::TILE_SIZE;
            float halfCamX = m_camera.getSize().x / 2.0f;
            float halfCamY = m_camera.getSize().y / 2.0f;

            if(mapWidth > m_camera.getSize().x){
                targetCenter.x = std::clamp(targetCenter.x, halfCamX, mapWidth - halfCamX);
            }else{
                targetCenter.x = mapWidth / 2.0f;
            }

            if(mapHeight > m_camera.getSize().y){
                targetCenter.y = std::clamp(targetCenter.y, halfCamY, mapHeight - halfCamY);
            }else{
                targetCenter.y = mapHeight / 2.0f;
            }
        }
        sf::Vector2f currentCenter = m_camera.getCenter();
        currentCenter += (targetCenter - currentCenter) * 5.0f * deltaTime.asSeconds();
        m_camera.setCenter(currentCenter);

        if(m_player->getPosition() != m_lastSentPosition || m_heartbeatTimer.getElapsedTime().asSeconds() > 1.0f){
            sf::Packet packet;
            packet << PacketType::PlayerPosition << m_player->getId() << m_player->getPosition();

            if(m_serverAddress){
                (void)m_socket.send(packet, m_serverAddress.value(), Config::SERVER_PORT);
            }

            m_lastSentPosition = m_player->getPosition();
            m_heartbeatTimer.restart();
        }
    }

    if(m_projectileManager){
        std::uint32_t myId = m_player ? m_player->getId() : 0;
        auto hitEnemies = m_projectileManager->update(deltaTime, m_enemies, m_map, myId);

        for(std::uint32_t enemyId : hitEnemies){
            if(m_serverAddress){
                sf::Packet hitPacket;
                hitPacket << PacketType::EnemyHit << enemyId << myId;
                (void)m_socket.send(hitPacket, m_serverAddress.value(), Config::SERVER_PORT);
            }
        }
    }
}

void ClientEngine::render(){
    m_window.clear(sf::Color(30, 30, 30));
    m_window.setView(m_camera);

    if(m_mapRenderer) m_mapRenderer->render(m_window);
    if(m_projectileManager) m_projectileManager->render(m_window);
    if(m_player) m_player->render(m_window);

    for(const auto& [id, otherPlayer]: m_otherPlayers){
        otherPlayer->render(m_window);
    }

    for(const auto& [id, enemy]: m_enemies){
        enemy->render(m_window);
    }

    m_window.setView(m_window.getDefaultView());

    renderUI();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void ClientEngine::renderUI(){
    ImGui::Begin("Swarm Inasion - Debug Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if(!m_player){
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(255, 0, 0, 255)), "DISCONECTED");

        ImGui::Text("Choose your class:");
        int classChoice = static_cast<int>(m_selectedClass);
        ImGui::RadioButton("Soldier", &classChoice, 0);
        ImGui::RadioButton("Scout", &classChoice, 1);
        ImGui::RadioButton("Tank", &classChoice, 2);
        ImGui::RadioButton("Medic", &classChoice, 3);
        m_selectedClass = static_cast<PlayerClass>(classChoice);

        if(ImGui::Button("Join the game", ImVec2(200, 50))){
            if(m_serverAddress){
                sf::Packet joinPacket;
                joinPacket << PacketType::JoinRequest << m_selectedClass;
                (void)m_socket.send(joinPacket, m_serverAddress.value(), Config::SERVER_PORT);
                std::cout << "[CLIENT] Request to join has been sent...\n";
            }
        }
    }else{
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(0, 255, 0, 255)), "CONNECTED! ID: %u", m_player->getId());
    }

    if(ImGui::Button("Send PING to server!")){
        sf::Packet packet;

        if(m_serverAddress){
            packet << PacketType::Ping << "Hello Server, are you there?";
            if(m_socket.send(packet, m_serverAddress.value(), Config::SERVER_PORT) != sf::Socket::Status::Done){
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
        m_mapRenderer = std::make_unique<MapRenderer>(m_map, Config::TILE_SIZE);
    }

    ImGui::End();
}