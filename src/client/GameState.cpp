#include "GameState.hpp"
#include "LobbyState.hpp"
#include <iostream>

GameState::GameState(ClientEngine& engine, std::uint32_t myPlayerId, PlayerClass myClass) : State(engine){
    m_camera.setSize({static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)});

    m_map = std::make_shared<MapGenerator>(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES);
    m_map->generate(1337);
    m_mapRenderer = std::make_unique<MapRenderer>(m_map, Config::TILE_SIZE);
    m_projectileManager = std::make_unique<ProjectileManager>();

    sf::Vector2f newPos = sf::Vector2f(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES) * Config::TILE_SIZE / 2.0f;
    m_player = std::make_unique<Player>(myPlayerId, newPos, myClass);
    m_lastSentPosition = sf::Vector2f(INFINITY, INFINITY);

    m_camera.setCenter(newPos);
}

void GameState::handleInput(const sf::Event& event){
    if(const auto& mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()){
        if(m_player){
            sf::Vector2i pixelPos(mouseBtn->position.x, mouseBtn->position.y);

            auto& window = m_engine.getWindow();
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, m_camera); 

            WeaponType myWeapon = HeroRegistry::getStats(m_player->getClass()).defaultWeapon;
            m_projectileManager->spawnProjectile(m_player->getId(), m_player->getPosition(), worldPos, myWeapon);

            if(m_engine.getServerAddress()){
                sf::Packet shootPacket;
                shootPacket << PacketType::PlayerShoots << m_player->getId() << myWeapon << m_player->getPosition() << worldPos;
                (void)m_engine.getSocket().send(shootPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            }
        }
    }
}

void GameState::handlePacket(PacketType type, sf::Packet& packet){
    m_lastServerMessageTimer.restart();

    if(type == PacketType::WorldState){
        handleWorldState(packet);
    }
    else if(type == PacketType::PlayerShoots){
        std::uint32_t shooterId;
        WeaponType weaponUsed;
        sf::Vector2f startPos, targetPos;
        if(packet >> shooterId >> weaponUsed >> startPos >> targetPos){
            if(m_projectileManager)
                m_projectileManager->spawnProjectile(shooterId, startPos, targetPos, weaponUsed);
        }
    }
    else if(type == PacketType::PlayerDied){
        std::cout<< "[CLIENT] You died! Leaving the game... \n";
        m_player.reset();
        m_otherPlayers.clear();
        m_enemies.clear();
    }
}

void GameState::handleWorldState(sf::Packet& packet){
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

void GameState::update(sf::Time deltaTime){
    if(m_player){
        if(m_lastServerMessageTimer.getElapsedTime().asSeconds() > Config::NETWORK_TIMEOUT_SECONDS){
            std::cout << "[CLIENT] Lost connection to the server (Timeout)!\n";
            m_player.reset();
            m_otherPlayers.clear();
            m_enemies.clear();
            return;
        }

        m_player->setFocused(m_engine.getWindow().hasFocus());
        m_player->update(deltaTime, m_map);

        auto view = m_engine.getWindow().getView();
        sf::Vector2f targetCenter = m_player->getPosition();

        if(m_map){
            float mapWidth = m_map->getWidth() * Config::TILE_SIZE;
            float mapHeight = m_map->getHeight() * Config::TILE_SIZE;
            float halfCamX = m_camera.getSize().x / 2.0f;
            float halfCamY = m_camera.getSize().y / 2.0f;

            if(mapWidth > m_camera.getSize().x) {
                targetCenter.x = std::clamp(targetCenter.x, halfCamX, mapWidth - halfCamX);
            } else {
                targetCenter.x = mapWidth / 2.0f;
            } 
            
            if(mapHeight > m_camera.getSize().y) {
                targetCenter.y = std::clamp(targetCenter.y, halfCamY, mapHeight - halfCamY);
            } else {
                targetCenter.y = mapHeight / 2.0f;
            } 
        }

        sf::Vector2f currentCenter = m_camera.getCenter();
        currentCenter += (targetCenter - currentCenter) * 5.0f * deltaTime.asSeconds();
        m_camera.setCenter(currentCenter);

        if(m_player->getPosition() != m_lastSentPosition || m_heartbeatTimer.getElapsedTime().asSeconds() > 1.0f){
            sf::Packet packet;
            packet << PacketType::PlayerPosition << m_player->getId() << m_player->getPosition();

            if(m_engine.getServerAddress()){
                (void)m_engine.getSocket().send(packet, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            }

            m_lastSentPosition = m_player->getPosition();
            m_heartbeatTimer.restart();
        }
    }

    if(m_projectileManager){
        std::uint32_t myId = m_player ? m_player->getId() : 0;
        auto hitEnemies = m_projectileManager->update(deltaTime, m_enemies, m_map, myId);

        for(std::uint32_t enemyId : hitEnemies){
            if(m_engine.getServerAddress()){
                sf::Packet hitPacket;
                hitPacket << PacketType::EnemyHit << enemyId << myId;
                (void)m_engine.getSocket().send(hitPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            }
        }
    }
}

void GameState::render() {
    auto& window = m_engine.getWindow();

    window.setView(m_camera);

    if(m_mapRenderer) m_mapRenderer->render(window);
    if(m_projectileManager) m_projectileManager->render(window);
    if(m_player) m_player->render(window);

    for(const auto& [id, otherPlayer]: m_otherPlayers){
        otherPlayer->render(window);
    }

    for(const auto& [id, enemy]: m_enemies){
        enemy->render(window);
    }

    window.setView(window.getDefaultView());
}

void GameState::renderUI(){
    ImGui::Begin("Swarm Invasion - Debug Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if(ImGui::Button("Disconnect/Return to Lobby", ImVec2(-1, 30))){
        m_engine.changeState(std::make_unique<LobbyState>(m_engine));
    }
    ImGui::End();
}