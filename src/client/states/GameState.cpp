#include "GameState.hpp"
#include "LobbyState.hpp"
#include "../entities/Medic.hpp"
#include <iostream>

GameState::GameState(ClientEngine& engine, std::uint32_t myPlayerId, PlayerClass myClass) : State(engine){
    m_camera.setSize({static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)});

    m_map = std::make_shared<MapGenerator>(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES);
    m_map->generate(1337);
    m_mapRenderer = std::make_unique<MapRenderer>(m_map, Config::TILE_SIZE);
    m_mapRenderer->rebuild();
    m_projectileManager = std::make_unique<ProjectileManager>();

    sf::Vector2f newPos = sf::Vector2f(Config::MAP_WIDTH_TILES, Config::MAP_HEIGHT_TILES) * Config::TILE_SIZE / 2.0f;
    m_player = Player::create(myPlayerId, newPos, myClass);
    m_lastSentPosition = sf::Vector2f(INFINITY, INFINITY);

    m_camera.setCenter(newPos);
}

GameState::~GameState(){
    if(m_player && m_engine.getServerAddress()){
        sf::Packet packet;
        packet << PacketType::PlayerDisconnect << m_player->getId();
        (void)m_engine.getSocket().send(packet, m_engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

void GameState::handleInput(const sf::Event& event){
    if(m_isChoosingUpgrade) return;

    if(const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()){
        if(m_player){
            sf::Vector2i pixelPos(mouseBtn->position.x, mouseBtn->position.y);
            auto& window = m_engine.getWindow();
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, m_camera); 

            if(mouseBtn->button == sf::Mouse::Button::Right){
                m_player->onRMB(worldPos, m_engine, *m_projectileManager);
            }

        }
    }

    if(const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()){
        if(m_player) {
            auto& window = m_engine.getWindow();
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, m_camera);

            if(keyEvent->code == sf::Keyboard::Key::LShift) m_player->onShift(worldPos, m_engine, *m_projectileManager);
            if(keyEvent->code == sf::Keyboard::Key::Q) m_player->onQ(worldPos, m_engine, *m_projectileManager);
            if(keyEvent->code == sf::Keyboard::Key::E) m_player->onE(worldPos, m_engine, *m_projectileManager);
            if(keyEvent->code == sf::Keyboard::Key::R) m_player->reload();
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
                m_projectileManager->spawnProjectile(shooterId, startPos, targetPos, weaponUsed, Faction::Players);
        }
    }
    else if(type == PacketType::PlayerDied){
        std::cout<< "[CLIENT] You died! Leaving the game... \n";
        m_player.reset();
        m_otherPlayers.clear();
        m_enemies.clear();
    }
    else if(type == PacketType::LevelUpTriggered){
        m_isChoosingUpgrade = true;
        m_myChoice = -1;
        m_clientUpgradeTimer.restart();
    }
    else if(type == PacketType::EnemyShoots){
        WeaponType weapon;
        sf::Vector2f startPos, targetPos;
        if(packet >> weapon >> startPos >> targetPos){
            if(m_projectileManager){
                m_projectileManager->spawnProjectile(-1, startPos, targetPos, weapon, Faction::Enemies);
            }
        }
    }
    else if(type == PacketType::SpawnHealField){
        std::uint32_t id;
        sf::Vector2f pos;
        float radius, duration;
        if(packet >> id >> pos >> radius >> duration){
            m_healFields[id] = std::make_unique<HealField>(id, pos, duration, radius);
        }
    }
    else if(type == PacketType::SpawnBlackHole){
        std::uint32_t id;
        sf::Vector2f pos;
        float duration;
        if(packet >> id >> pos >> duration){
            m_blackHoles[id] = std::make_unique<BlackHole>(id, pos, duration);
        }
    }
    else if(type == PacketType::SpawnDecoy){
        std::uint32_t id;
        sf::Vector2f pos;
        float maxHp;
        if(packet >> id >> pos >> maxHp){
            m_decoys[id] = std::make_unique<Decoy>(id, pos, maxHp);
        }
    }
    else if(type == PacketType::SpawnMedicOrb){
        std::uint32_t id;
        sf::Vector2f pos;
        sf::Vector2f direction;
        if(packet >> id >> pos >> direction){
            m_medicOrbs[id] = std::make_unique<MedicOrb>(id, pos, direction);
        }
    }
    else if(type == PacketType::SpawnMedicBarrier){
        std::uint32_t id;
        sf::Vector2f center;
        float facingAngle;
        float duration;
        if(packet >> id >> center >> facingAngle >> duration){
            m_medicBarriers[id] = std::make_unique<MedicBarrier>(id, center, facingAngle, duration);
        }
    }
    else if(type == PacketType::SpawnMedicDrone){
        std::uint32_t id;
        std::uint32_t ownerId;
        sf::Vector2f pos;
        if(packet >> id >> ownerId >> pos){
            m_medicDrones[id] = std::make_unique<MedicDrone>(id, ownerId, pos);
        }
    }
    else if(type == PacketType::DroneShoots){
        sf::Vector2f startPos;
        sf::Vector2f targetPos;
        if(packet >> startPos >> targetPos){
            if(m_projectileManager){
                m_projectileManager->spawnProjectile(0, startPos, targetPos, WeaponType::DroneBlaster, Faction::Players);
            }
        }
    }
    else if(type == PacketType::DecoyExplode){
        std::uint32_t id;
        if(packet >> id){
            if (m_decoys.count(id)) {
                m_decoys.at(id)->triggerExplosion();
            }
        }
    }
    else if(type == PacketType::PlayerDealtDamage){
        float damage;
        if(packet >> damage && m_player){
            m_player->addUltCharge(damage);
        }
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
        float stealthTimer;
        packet >> id >> pClass >> pos >> hp >> stealthTimer;

        activeServerIds.push_back(id);

        if(m_player && id == m_player->getId()){
            m_player->setHp(hp);
            m_player->setStealthTimer(stealthTimer);
            continue;
        }

        if(m_otherPlayers.find(id) == m_otherPlayers.end()){
            m_otherPlayers[id] = Player::create(id, pos, pClass);
        }

        m_otherPlayers[id]->setHp(hp);
        m_otherPlayers[id]->setPosition(pos);
        m_otherPlayers[id]->setStealthTimer(stealthTimer);
    }

    bool serverIsPaused;
    packet >> m_teamLevel >> m_teamExp >> m_teamExpMax >> serverIsPaused;

    if(!serverIsPaused && m_isChoosingUpgrade){
        m_isChoosingUpgrade = false;
        m_myChoice = -1;
    }

    if(serverIsPaused && !m_isChoosingUpgrade){
        m_isChoosingUpgrade = true;
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
    
    // --- ENERGY CELLS ---
    std::uint32_t cellCount;
    if(!(packet >> cellCount)) return;

    m_energyCells.clear();
    for(std::uint32_t i = 0; i < cellCount; i++){
        std::uint32_t cId;
        sf::Vector2f cPos;
        packet >> cId >> cPos;
        m_energyCells[cId] = cPos;
    }

    std::uint32_t decoyCount;
    if(!(packet >> decoyCount)) return;

    std::vector<std::uint32_t> activeDecoyIds;
    for(std::uint32_t i = 0; i < decoyCount; i++){
        std::uint32_t dId;
        sf::Vector2f dPos;
        float dHp;
        float dMaxHp;
        packet >> dId >> dPos >> dHp >> dMaxHp;

        activeDecoyIds.push_back(dId);

        if(m_decoys.find(dId) == m_decoys.end()){
            m_decoys[dId] = std::make_unique<Decoy>(dId, dPos, dMaxHp);
        }

        m_decoys[dId]->setPosition(dPos);
        m_decoys[dId]->setHp(dHp);
        m_decoys[dId]->setMaxHp(dMaxHp);
    }

    for(auto it = m_decoys.begin(); it != m_decoys.end();){
        bool isActive = std::find(activeDecoyIds.begin(), activeDecoyIds.end(), it->first) != activeDecoyIds.end();
        if(!isActive && !it->second->isExploding()){
            it = m_decoys.erase(it);
        }else{
            ++it;
        }
    }

    std::uint32_t orbCount;
    if(!(packet >> orbCount)) return;

    std::vector<std::uint32_t> activeOrbIds;
    for(std::uint32_t i = 0; i < orbCount; i++){
        std::uint32_t oId;
        sf::Vector2f oPos;
        packet >> oId >> oPos;

        activeOrbIds.push_back(oId);

        if(m_medicOrbs.find(oId) == m_medicOrbs.end()){
            m_medicOrbs[oId] = std::make_unique<MedicOrb>(oId, oPos, sf::Vector2f(1.0f, 0.0f));
        }

        m_medicOrbs[oId]->setServerPosition(oPos);
    }

    for(auto it = m_medicOrbs.begin(); it != m_medicOrbs.end();){
        if(std::find(activeOrbIds.begin(), activeOrbIds.end(), it->first) == activeOrbIds.end()){
            it = m_medicOrbs.erase(it);
        }else{
            ++it;
        }
    }

    std::uint32_t droneCount;
    if(!(packet >> droneCount)) return;

    std::vector<std::uint32_t> activeDroneIds;
    bool myHasDrone = false;
    float myDroneLifetime = 0.0f;

    for(std::uint32_t i = 0; i < droneCount; i++){
        std::uint32_t dId;
        std::uint32_t ownerId;
        sf::Vector2f dPos;
        MedicDroneState dState;
        float dLifetime;
        packet >> dId >> ownerId >> dPos >> dState >> dLifetime;

        activeDroneIds.push_back(dId);

        if(m_medicDrones.find(dId) == m_medicDrones.end()){
            m_medicDrones[dId] = std::make_unique<MedicDrone>(dId, ownerId, dPos);
        }

        m_medicDrones[dId]->setServerPosition(dPos);
        m_medicDrones[dId]->setDroneState(dState);
        m_medicDrones[dId]->setHp(dLifetime);

        if(m_player && ownerId == m_player->getId()){
            myHasDrone = true;
            myDroneLifetime = dLifetime;
        }
    }

    for(auto it = m_medicDrones.begin(); it != m_medicDrones.end();){
        if(std::find(activeDroneIds.begin(), activeDroneIds.end(), it->first) == activeDroneIds.end()){
            it = m_medicDrones.erase(it);
        }else{
            ++it;
        }
    }

    if(m_player && m_player->getClass() == PlayerClass::Medic){
        static_cast<Medic*>(m_player.get())->setDroneState(myHasDrone, myDroneLifetime);
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

        if(!m_isChoosingUpgrade){
            m_player->update(deltaTime, m_map);
        }

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
        if(!m_isChoosingUpgrade){
            std::vector<Entity*> collisionTargets;

            if(m_player) collisionTargets.push_back(m_player.get());
            for(auto& [id, otherPlayer] : m_otherPlayers) collisionTargets.push_back(otherPlayer.get());
            for(auto& [id, enemy] : m_enemies) collisionTargets.push_back(enemy.get());
            for(auto& [id, decoy] : m_decoys){
                if(!decoy->isExploding()) collisionTargets.push_back(decoy.get());
            }

            std::vector<SectorBarrierSnapshot> barrierSnapshots;
            barrierSnapshots.reserve(m_medicBarriers.size());
            const float halfSpan = Config::MEDIC_BARRIER_SPAN / 2.0f;
            for (const auto& [id, barrier] : m_medicBarriers) {
                (void)id;
                barrierSnapshots.push_back({
                    barrier->getCenter(),
                    barrier->getFacingAngle(),
                    Config::MEDIC_BARRIER_ARC_RADIUS,
                    halfSpan,
                    Config::MEDIC_BARRIER_WALL_THICKNESS
                });
            }

            auto hits = m_projectileManager->update(deltaTime, collisionTargets, m_map, barrierSnapshots);

            for(const auto& hit : hits){
                if(!m_engine.getServerAddress()) break;
                if (hit.weapon == WeaponType::MedicNeedle || hit.weapon == WeaponType::DroneBlaster) continue;
                sf::Packet hitPacket;
                hitPacket << PacketType::EntityHit << hit.shooterId << hit.targetId << hit.weapon;
                (void)m_engine.getSocket().send(hitPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            }

            if(m_player){
                auto abilityHits = m_player->checkAbilityHits(collisionTargets);

                for(const auto& hit : abilityHits){
                    if(!m_engine.getServerAddress()) break;

                    sf::Packet hitPacket;
                    hitPacket << PacketType::AbilityHit << m_player->getId() << hit.targetId << hit.ability;
                    (void)m_engine.getSocket().send(hitPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
                }
            }
        }
    }

    for (auto it = m_healFields.begin(); it != m_healFields.end(); ) {
        auto& field = it->second;
        field->update(deltaTime, m_map);
        if (field->getHp() <= 0.0f) it = m_healFields.erase(it);
        else ++it;
    }

    for (auto it = m_blackHoles.begin(); it != m_blackHoles.end(); ) {
        auto& blackHole = it->second;
        blackHole->update(deltaTime, m_map);
        if (blackHole->getHp() <= 0.0f) it = m_blackHoles.erase(it);
        else ++it;
    }

    for (auto it = m_decoys.begin(); it != m_decoys.end(); ) {
        if (it->second->isFinished()) {
            it = m_decoys.erase(it);
        } else {
            it->second->update(deltaTime, m_map);
            ++it;
        }
    }

    std::vector<Entity*> allies;
    std::vector<Entity*> enemyTargets;
    if (m_player) allies.push_back(m_player.get());
    for (auto& [id, otherPlayer] : m_otherPlayers) allies.push_back(otherPlayer.get());
    for (auto& [id, enemy] : m_enemies) enemyTargets.push_back(enemy.get());

    for (auto it = m_medicOrbs.begin(); it != m_medicOrbs.end(); ) {
        it->second->setNearbyEntities(allies, enemyTargets);
        it->second->update(deltaTime, m_map);
        if (it->second->getHp() <= 0.0f) {
            it = m_medicOrbs.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_medicBarriers.begin(); it != m_medicBarriers.end(); ) {
        it->second->update(deltaTime, m_map);
        if (it->second->getHp() <= 0.0f) {
            it = m_medicBarriers.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_medicDrones.begin(); it != m_medicDrones.end(); ) {
        it->second->update(deltaTime, m_map);
        ++it;
    }

    if (m_player && m_engine.getWindow().hasFocus()) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || m_player->isAutoFiring()) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(m_engine.getWindow());
            sf::Vector2f worldPos = m_engine.getWindow().mapPixelToCoords(pixelPos, m_camera);

            m_player->onLMB(worldPos, m_engine, *m_projectileManager, m_enemies);
        }
    }
}

void GameState::render() {
    auto& window = m_engine.getWindow();

    window.setView(m_camera);

    if(m_mapRenderer) m_mapRenderer->render(window);

    for(const auto& [id, field] : m_healFields){
        field->render(window);
    }

    for(const auto& [id, blackHole] : m_blackHoles){
        blackHole->render(window);
    }

    for(const auto& [id, decoy] : m_decoys){
        decoy->render(window);
    }

    for(const auto& [id, orb] : m_medicOrbs){
        orb->render(window);
    }

    for(const auto& [id, barrier] : m_medicBarriers){
        barrier->render(window);
    }

    for(const auto& [id, drone] : m_medicDrones){
        drone->render(window);
    }
    
    sf::CircleShape crystalShape(4.0f, 4);
    crystalShape.setFillColor(sf::Color::Green);
    crystalShape.setOrigin({4.0f, 4.0f});

    for(const auto& [id, pos] : m_energyCells){
        crystalShape.setPosition(pos);
        window.draw(crystalShape);
    }

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
    // --- DEATH SCREEN ---
    if(!m_player){
        ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH / 2.0f - 150.0f, Config::WINDOW_HEIGHT / 2.0f - 100.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f), ImGuiCond_Always);
        ImGui::Begin("YOU DIED", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "The swarm consumed you...");
        ImGui::Dummy(ImVec2(0.0f, 40.0f));

        if(ImGui::Button("Return to Lobby", ImVec2(-1, 50))){
            m_engine.changeState(std::make_unique<LobbyState>(m_engine));
        }
        ImGui::End();
        return;
    }

    if(m_isChoosingUpgrade){
        ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH/2.0f - 300.0f, Config::WINDOW_HEIGHT/2.0f - 200.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(600, 400));
        ImGui::Begin("CHOOSE YOUR AUGMENT", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        float timePassed = m_clientUpgradeTimer.getElapsedTime().asSeconds();
        float progress = 1.0f - (timePassed / Config::LEVEL_UP_TIMEOUT);
        ImGui::ProgressBar(progress, ImVec2(-1, 20), "Time Remaining");

        ImGui::Columns(3, "Cards", true);
        for(int i = 0; i < 3; i++){
            bool isSelected = (m_myChoice == i);
            bool pushedColor = false;

            if(isSelected){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                pushedColor = true;
            }else if(m_myChoice != -1){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.6f));
                pushedColor = true;
            } 
                

            char label[32];
            sprintf(label, "CARD %d", i + 1);
            if(ImGui::Button(label, ImVec2(-1, 200))){
                m_myChoice = i;
                sf::Packet packet;
                packet << PacketType::CardSelected << m_player->getId() << m_myChoice;
                (void)m_engine.getSocket().send(packet, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            }

            if(pushedColor) ImGui::PopStyleColor();
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::End();
    }


    float expProgress = static_cast<float>(m_teamExp) / m_teamExpMax;
    char expOverlay[64];
    sprintf(expOverlay, "LEVEL %d (%d/%d)", m_teamLevel, m_teamExp, m_teamExpMax);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
    ImGui::ProgressBar(expProgress, ImVec2(-1.0f, 24.0f), expOverlay);
    ImGui::PopStyleColor();
    ImGui::Separator();
    
    if(m_player) m_player->renderUI();

    ImGui::Begin("Swarm Invasion - Debug Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if(ImGui::Button("Disconnect/Return to Lobby", ImVec2(-1, 30))){
        m_engine.changeState(std::make_unique<LobbyState>(m_engine));
    }
    ImGui::End();
}