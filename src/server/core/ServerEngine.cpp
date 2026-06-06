#include "ServerEngine.hpp"
#include "SectorMath.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

struct SimpleProfiler {
    std::string name;
    std::chrono::high_resolution_clock::time_point start;

    SimpleProfiler(std::string n) : name(n) {
        start = std::chrono::high_resolution_clock::now();
    }
    ~SimpleProfiler() {
        auto end = std::chrono::high_resolution_clock::now();
        float ms = std::chrono::duration<float, std::milli>(end - start).count();
        
        if (ms > 1.0f) {
            std::cout << "[PROFILER] " << name << " zajelo: " << ms << " ms\n";
        }
    }
};
#define PROFILE_BLOCK(name) SimpleProfiler profiler_##__LINE__(name)

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
                case PacketType::AbilityHit:        handleAbilityHit(packet); break;
                case PacketType::AbilityUsed:       handleAbilityUsed(packet); break;
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

    for(auto& [id, info] : m_clients) {
        if (info.invTimer > 0.0f) info.invTimer -= deltaTime.asSeconds();
        if (info.stealthTimer > 0.0f) info.stealthTimer -= deltaTime.asSeconds();
    }

    for (auto it = m_decoys.begin(); it != m_decoys.end(); ) {
        it->second.lifetime -= deltaTime.asSeconds();
        if (it->second.lifetime <= 0.0f) {
            explodeDecoy(it->first, it->second);
            it = m_decoys.erase(it);
        } else {
            ++it;
        }
    }

    // SERVER RESET
    if(m_clients.empty() && !m_enemies.empty()){
        m_enemies.clear();
        std::cout << "[SERVER] All players left. Resetting world...\n";
    }

    // ENEMIES DIRECTOR
    if(!m_clients.empty()){
        PROFILE_BLOCK("AI_AND_MOVEMENT");

        std::map<std::uint32_t, ClientInfo> visibleTargets;
        
        // Dodajemy tylko widocznych graczy
        for(const auto& [id, info] : m_clients) {
            if (info.stealthTimer <= 0.0f) {
                visibleTargets.insert({id, info});
            }
        }
        
        for(const auto& [id, decoy] : m_decoys) {
            if (m_clients.count(decoy.ownerId)) {
                ClientInfo fakePlayer = m_clients.at(decoy.ownerId);
                
                fakePlayer.position = decoy.pos;
                fakePlayer.hp = decoy.hp; 
                fakePlayer.invTimer = 0.0f;
                fakePlayer.stealthTimer = 0.0f;
                fakePlayer.pClass = PlayerClass::Vanguard;
                
                visibleTargets.insert({id + 100000, fakePlayer});
            }
        }

        m_aiDirector.updateWaves(deltaTime, m_enemies, visibleTargets, m_map, m_globalEntityCounter);

        std::vector<std::uint32_t> deadFromPoison;
        float dt = deltaTime.asSeconds();
        for (auto& [id, enemy] : m_enemies) {
            enemy->tickStatusEffects(dt);
            if (enemy->getHp() <= 0.0f) deadFromPoison.push_back(id);
        }
        for (std::uint32_t enemyId : deadFromPoison) {
            if (m_enemies.count(enemyId)) {
                m_energyCells[m_globalEntityCounter++] = {m_enemies[enemyId]->getPosition(), 1, 0};
                m_enemies.erase(enemyId);
            }
        }
        
        std::vector<EnemyShootEvent> shootEvents;
        
        auto deadPlayers = m_aiDirector.updateBehaviours(deltaTime, m_enemies, visibleTargets, m_map, shootEvents);

        for(auto& [id, info] : m_clients){
            if(visibleTargets.count(id)) info.hp = visibleTargets.at(id).hp;
        }
        for(auto& [id, decoy] : m_decoys){
            if(visibleTargets.count(id + 100000)) decoy.hp = visibleTargets.at(id + 100000).hp;
        }

        for(std::uint32_t deadId : deadPlayers){
            if (deadId >= 100000) {
                std::uint32_t actualDecoyId = deadId - 100000;
                if (m_decoys.count(actualDecoyId)) {
                    DecoyData decoy = m_decoys.at(actualDecoyId);
                    explodeDecoy(actualDecoyId, decoy);
                    m_decoys.erase(actualDecoyId);
                }
            } 
            else if (m_clients.count(deadId)) {
                auto& targetInfo = m_clients.at(deadId);
                std::cout << "[SERVER] Player " << deadId << " died!\n";

                sf::Packet deathPacket;
                deathPacket << PacketType::PlayerDied;
                (void)m_socket.send(deathPacket, targetInfo.ip, targetInfo.port);

                for (auto dit = m_medicDrones.begin(); dit != m_medicDrones.end(); ) {
                    if (dit->second.ownerId == deadId) dit = m_medicDrones.erase(dit);
                    else ++dit;
                }

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

    // HEALING FIELDS
    updateHealingFields(deltaTime);

    updateMedicPassives(deltaTime);

    updateBlackHoles(deltaTime);

    updateMedicOrbs(deltaTime);

    updateMedicBarriers(deltaTime);

    updateMedicDrones(deltaTime);

    updateServerProjectiles(deltaTime);

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
    std::uint32_t shooterId, targetId;
    WeaponType weaponUsed;

    if(packet >> shooterId >> targetId >> weaponUsed){
        if (weaponUsed == WeaponType::MedicNeedle) return;

        float baseDamage = WeaponRegistry::getStats(weaponUsed).damage;

        if(m_decoys.count(targetId)){
            auto& decoy = m_decoys.at(targetId);
            decoy.hp -= baseDamage;
            if(decoy.hp <= 0.0f){
                DecoyData copy = decoy;
                explodeDecoy(targetId, copy);
                m_decoys.erase(targetId);
            }
            return;
        }

        // Player hit
        if(m_clients.count(targetId)){
            if(m_clients.at(targetId).invTimer > 0.0f) return;
            float damageTaken = baseDamage;

            if(m_clients.at(targetId).pClass == PlayerClass::Juggernaut) damageTaken *= 0.8f;

            m_clients.at(targetId).hp -= damageTaken;

            if(m_clients.at(targetId).hp <= 0.0f){
                sf::Packet deathPacket;
                deathPacket << PacketType::PlayerDied;
                (void)m_socket.send(deathPacket, m_clients.at(targetId).ip, m_clients.at(targetId).port);
            }
        }
        // Enemy hit
        if(m_enemies.count(targetId)){
            float damageDealt = baseDamage;

            if (m_clients.count(shooterId) && m_clients.at(shooterId).pClass == PlayerClass::Soldier) {
                if ((std::rand() % 100) < 20) {
                    damageDealt *= 2.0f;
                }
            }

            m_enemies[targetId]->takeDamage(damageDealt);

            if(m_clients.count(shooterId) && weaponUsed != WeaponType::VanguardWave){
                sf::Packet damagePacket;
                damagePacket << PacketType::PlayerDealtDamage << damageDealt;
                (void)m_socket.send(damagePacket, m_clients.at(shooterId).ip, m_clients.at(shooterId).port);
            }

            if(m_enemies[targetId]->getHp() <= 0.0f){
                if (m_clients.count(shooterId) && m_clients.at(shooterId).pClass == PlayerClass::Vanguard) {
                    float maxHp = HeroRegistry::getStats(PlayerClass::Vanguard).maxHp;
                    m_clients.at(shooterId).hp += 5.0f;
                    if (m_clients.at(shooterId).hp > maxHp) m_clients.at(shooterId).hp = maxHp;
                }

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
        if (weaponUsed == WeaponType::MedicNeedle && m_clients.count(shooterId)) {
            spawnMedicNeedle(shooterId, startPos, targetPos);
        }

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
        for (auto it = m_medicDrones.begin(); it != m_medicDrones.end(); ) {
            if (it->second.ownerId == playerId) it = m_medicDrones.erase(it);
            else ++it;
        }
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

void ServerEngine::handleAbilityHit(sf::Packet& packet){
    std::uint32_t shooterId, targetId;
    AbilityType ability;
    if(packet >> shooterId >> targetId >> ability){
        if(ability == AbilityType::JuggernautDash){
            float damage = 20.0f;

            if(!m_enemies.count(targetId)) return;
            m_enemies[targetId]->takeDamage(damage);

            if(m_clients.count(shooterId)){
                sf::Vector2f pushDir = m_enemies[targetId]->getPosition() - m_clients.at(shooterId).position;
                float lenSq = pushDir.lengthSquared();

                if(lenSq > 0.0f){
                    pushDir /= std::sqrt(lenSq);
                    m_enemies[targetId]->applyKnockback(pushDir, 1200.0f);
                }

                sf::Packet damagePacket;
                damagePacket << PacketType::PlayerDealtDamage << damage;
                (void)m_socket.send(damagePacket, m_clients.at(shooterId).ip, m_clients.at(shooterId).port);
            }
        }else if(ability == AbilityType::JuggernautRepulsor){
            float damage = 10.0f;
            
            if(m_enemies.count(targetId)){
                m_enemies[targetId]->takeDamage(damage);

                if (m_clients.count(shooterId)) {
                    sf::Vector2f pushDir = m_enemies[targetId]->getPosition() - m_clients.at(shooterId).position;
                    m_enemies[targetId]->applyKnockback(pushDir, 1000.0f);

                    sf::Packet damagePacket;
                    damagePacket << PacketType::PlayerDealtDamage << damage;
                    (void)m_socket.send(damagePacket, m_clients.at(shooterId).ip, m_clients.at(shooterId).port);
                }
            }
        }else if(ability == AbilityType::VanguardKatanaSlash){
            const float damage = 35.0f;

            if (m_enemies.count(targetId)) {
                m_enemies[targetId]->takeDamage(damage);

                if (m_clients.count(shooterId)) {
                    sf::Vector2f pushDir = m_enemies[targetId]->getPosition() - m_clients.at(shooterId).position;
                    m_enemies[targetId]->applyKnockback(pushDir, 300.0f);

                    sf::Packet damagePacket;
                    damagePacket << PacketType::PlayerDealtDamage << damage;
                    (void)m_socket.send(damagePacket, m_clients.at(shooterId).ip, m_clients.at(shooterId).port);
                }

                if (m_enemies[targetId]->getHp() <= 0.0f) {
                    if (m_clients.count(shooterId) && m_clients.at(shooterId).pClass == PlayerClass::Vanguard) {
                        float maxHp = HeroRegistry::getStats(PlayerClass::Vanguard).maxHp;
                        m_clients.at(shooterId).hp += 5.0f;
                        if (m_clients.at(shooterId).hp > maxHp) m_clients.at(shooterId).hp = maxHp;
                    }

                    m_energyCells[m_globalEntityCounter++] = {m_enemies[targetId]->getPosition(), 1, 0};
                    m_enemies.erase(targetId);
                }
            }
        }else if(ability == AbilityType:: VanguardDash){
            float damage = 25.0f;

            if (m_enemies.count(targetId)) {
                m_enemies[targetId]->takeDamage(damage);

                if (m_clients.count(shooterId)) {
                    sf::Packet damagePacket;
                    damagePacket << PacketType::PlayerDealtDamage << damage;
                    (void)m_socket.send(damagePacket, m_clients.at(shooterId).ip, m_clients.at(shooterId).port);
                }

                if (m_enemies[targetId]->getHp() <= 0.0f) {
                    if (m_clients.count(shooterId) && m_clients.at(shooterId).pClass == PlayerClass::Vanguard) {
                        float maxHp = HeroRegistry::getStats(PlayerClass::Vanguard).maxHp;
                        m_clients.at(shooterId).hp += 5.0f;
                        if (m_clients.at(shooterId).hp > maxHp) m_clients.at(shooterId).hp = maxHp;
                    }

                    m_energyCells[m_globalEntityCounter++] = {m_enemies[targetId]->getPosition(), 1, 0};
                    m_enemies.erase(targetId);
                }
            }
        }
    }
}

void ServerEngine::handleAbilityUsed(sf::Packet& packet){
    std::uint32_t playerId;
    AbilityType ability;
    sf::Vector2f pos;
    if (packet >> playerId >> ability >> pos) {

        if(ability == AbilityType::SoldierHealField){
            std::uint32_t fieldId = m_globalEntityCounter++;
            m_healFields[fieldId] = {pos, 100.0f, 5.0f, 10.0f};

            sf::Packet broadcastPacket;
            broadcastPacket << PacketType::SpawnHealField << fieldId << pos << 100.0f << 5.0f;
            for (const auto& [id, clientInfo] : m_clients) {
                (void)m_socket.send(broadcastPacket, clientInfo.ip, clientInfo.port);
            }
        }else if(ability == AbilityType::JuggernautBlackHole){
            std::uint32_t bhId = m_globalEntityCounter++;
            m_blackHoles[bhId] = {pos, 4.0f, playerId};

            sf::Packet broadcastPacket;
            broadcastPacket << PacketType::SpawnBlackHole << bhId << pos << 3.0f;
            for (const auto& [id, clientInfo] : m_clients) {
                (void)m_socket.send(broadcastPacket, clientInfo.ip, clientInfo.port);
            }
        }else if(ability == AbilityType::VanguardDash){
            if (m_clients.count(playerId)) {
                m_clients.at(playerId).invTimer = 0.3f;
            }
        }else if(ability == AbilityType::VanguardDecoy){
            if (!m_clients.count(playerId)) return;

            auto& client = m_clients.at(playerId);
            if (client.stealthTimer > 0.0f || playerHasActiveDecoy(playerId)) return;

            client.stealthTimer = Config::VANGUARD_STEALTH_DURATION;

            std::uint32_t decoyId = m_globalEntityCounter++;
            m_decoys[decoyId] = {
                pos,
                Config::VANGUARD_DECOY_HP,
                playerId,
                Config::VANGUARD_STEALTH_DURATION
            };

            sf::Packet broadcastPacket;
            broadcastPacket << PacketType::SpawnDecoy << decoyId << pos << Config::VANGUARD_DECOY_HP;
            for (const auto& [id, clientInfo] : m_clients) {
                (void)m_socket.send(broadcastPacket, clientInfo.ip, clientInfo.port);
            }
        }else if(ability == AbilityType::MedicTeleport){
            if (!m_clients.count(playerId)) return;

            auto& client = m_clients.at(playerId);
            if (client.pClass != PlayerClass::Medic) return;

            sf::Vector2f origin = client.position;
            sf::Vector2f target = pos;
            sf::Vector2f delta = target - origin;
            float distSq = delta.lengthSquared();

            if (distSq > 1e-4f) {
                float dist = std::sqrt(distSq);
                float maxAllowed = Config::MEDIC_TELEPORT_RANGE + Config::MEDIC_TELEPORT_RANGE_TOLERANCE;
                if (dist > maxAllowed) {
                    target = origin + (delta / dist) * Config::MEDIC_TELEPORT_RANGE;
                }
            }

            const float radius = HeroRegistry::getStats(PlayerClass::Medic).radius;
            if (m_map->checkCollision(target, radius)) return;

            client.position = target;
            client.invTimer = Config::MEDIC_TELEPORT_IFRAMES;
        }else if(ability == AbilityType::MedicOrb){
            if (!m_clients.count(playerId)) return;

            auto& client = m_clients.at(playerId);
            if (client.pClass != PlayerClass::Medic) return;
            if (playerHasActiveOrb(playerId)) return;

            sf::Vector2f dir = pos;
            float lenSq = dir.lengthSquared();
            if (lenSq < 1e-4f) {
                dir = sf::Vector2f(1.0f, 0.0f);
            } else if (std::abs(lenSq - 1.0f) > 0.1f) {
                dir /= std::sqrt(lenSq);
            }

            std::uint32_t orbId = m_globalEntityCounter++;
            sf::Vector2f spawnPos = client.position;
            m_medicOrbs[orbId] = {
                spawnPos,
                dir * Config::MEDIC_ORB_SPEED,
                Config::MEDIC_ORB_LIFETIME,
                0.0f,
                playerId
            };

            sf::Packet broadcastPacket;
            broadcastPacket << PacketType::SpawnMedicOrb << orbId << spawnPos << dir;
            for (const auto& [id, clientInfo] : m_clients) {
                (void)m_socket.send(broadcastPacket, clientInfo.ip, clientInfo.port);
            }
        }else if(ability == AbilityType::MedicBarrier){
            float facingAngle;
            if (!(packet >> facingAngle)) return;
            if (!m_clients.count(playerId)) return;

            auto& client = m_clients.at(playerId);
            if (client.pClass != PlayerClass::Medic) return;

            sf::Vector2f origin = client.position;
            facingAngle = SectorMath::normalizeAngle(facingAngle);
            sf::Vector2f arcCenter = SectorMath::arcCircleCenter(
                origin,
                facingAngle,
                Config::MEDIC_BARRIER_ARC_RADIUS,
                Config::MEDIC_BARRIER_STANDOFF);

            std::uint32_t barrierId = m_globalEntityCounter++;
            m_medicBarriers[barrierId] = {
                arcCenter,
                facingAngle,
                Config::MEDIC_BARRIER_LIFETIME,
                playerId
            };

            sf::Packet broadcastPacket;
            broadcastPacket << PacketType::SpawnMedicBarrier << barrierId << arcCenter << facingAngle << Config::MEDIC_BARRIER_LIFETIME;
            for (const auto& [id, clientInfo] : m_clients) {
                (void)m_socket.send(broadcastPacket, clientInfo.ip, clientInfo.port);
            }
        }else if(ability == AbilityType::MedicUltCommand){
            if (!m_clients.count(playerId)) return;
            if (m_clients.at(playerId).pClass != PlayerClass::Medic) return;
            handleMedicDroneCommand(playerId, pos);
        }
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

void ServerEngine::updateHealingFields(sf::Time deltaTime){
    for (auto it = m_healFields.begin(); it != m_healFields.end(); ) {
        auto& field = it->second;

        for (auto& [id, clientInfo] : m_clients) {
            float distSq = (clientInfo.position - field.position).lengthSquared();
            if (distSq <= (field.radius * field.radius)) {
                float maxHp = HeroRegistry::getStats(clientInfo.pClass).maxHp;
                clientInfo.hp += field.healPerSecond * deltaTime.asSeconds();
                if (clientInfo.hp > maxHp) {
                    clientInfo.hp = maxHp;
                }
            }
        }

        field.duration -= deltaTime.asSeconds();
        if (field.duration <= 0.0f) {
            it = m_healFields.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerEngine::updateMedicPassives(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();

    for (auto& [id, clientInfo] : m_clients) {
        (void)id;
        if (clientInfo.pClass != PlayerClass::Medic) continue;

        float maxHp = HeroRegistry::getStats(clientInfo.pClass).maxHp;
        if (clientInfo.hp >= maxHp) continue;

        clientInfo.hp += Config::MEDIC_PASSIVE_HEAL_PER_SECOND * dt;
        if (clientInfo.hp > maxHp) clientInfo.hp = maxHp;
    }
}

void ServerEngine::updateBlackHoles(sf::Time deltaTime){
    std::vector<std::uint32_t> deadFromBlackHole;

    for(auto it = m_blackHoles.begin(); it != m_blackHoles.end(); ){
        it->second.duration -= deltaTime.asSeconds();
        if(it->second.duration <= 0.0f){
            it = m_blackHoles.erase(it);
            continue;
        }

        float pullRadius = 300.0f;
        float coreRadius = 30.0f;

        for(auto& [enemyId, enemy] : m_enemies){
            sf::Vector2f diff = it->second.position - enemy->getPosition();
            float distSq = diff.lengthSquared();

            if(distSq > 0.0f && distSq < pullRadius * pullRadius){
                float dist = std::sqrt(distSq);

                if(dist < coreRadius){
                    float damageDealt = 50.0f * deltaTime.asSeconds();
                    enemy->takeDamage(damageDealt);

                    if (m_clients.count(it->second.ownerId)) {
                        sf::Packet damagePacket;
                        damagePacket << PacketType::PlayerDealtDamage << damageDealt;
                        (void)m_socket.send(damagePacket, m_clients.at(it->second.ownerId).ip, m_clients.at(it->second.ownerId).port);
                    }

                    if(enemy->getHp() <= 0.0f) deadFromBlackHole.push_back(enemyId);
                    
                }else{
                    sf::Vector2f pullStep = (diff / dist) * 170.0f * deltaTime.asSeconds();
                    sf::Vector2f newPos = enemy->getPosition() + pullStep;

                    if(!m_map->checkCollision(newPos, EnemyRegistry::getStats(enemy->getType()).radius)){
                        enemy->setPosition(newPos);
                    }
                }
            }
        }
        ++it;
    }

    for(auto id: deadFromBlackHole){
        if(m_enemies.count(id)){
            m_energyCells[m_globalEntityCounter++] = {m_enemies[id]->getPosition(), 1, 0};
            m_enemies.erase(id);
        }
    }
}

void ServerEngine::explodeDecoy(std::uint32_t decoyId, const DecoyData& decoy){
    const float radiusSq = Config::VANGUARD_DECOY_EXPLOSION_RADIUS * Config::VANGUARD_DECOY_EXPLOSION_RADIUS;
    for (auto& [eId, targetEnemy] : m_enemies) {
        if ((targetEnemy->getPosition() - decoy.pos).lengthSquared() < radiusSq) {
            targetEnemy->takeDamage(Config::VANGUARD_DECOY_EXPLOSION_DAMAGE);
        }
    }

    sf::Packet expPacket;
    expPacket << PacketType::DecoyExplode << decoyId;
    for (const auto& [cId, cInfo] : m_clients) {
        (void)m_socket.send(expPacket, cInfo.ip, cInfo.port);
    }
}

bool ServerEngine::playerHasActiveDecoy(std::uint32_t playerId) const{
    for (const auto& [id, decoy] : m_decoys) {
        if (decoy.ownerId == playerId) return true;
    }
    return false;
}

bool ServerEngine::playerHasActiveOrb(std::uint32_t playerId) const{
    for (const auto& [id, orb] : m_medicOrbs) {
        if (orb.ownerId == playerId) return true;
    }
    return false;
}

void ServerEngine::updateMedicOrbs(sf::Time deltaTime){
    float dt = deltaTime.asSeconds();
    const float effectRadiusSq = Config::MEDIC_ORB_EFFECT_RADIUS * Config::MEDIC_ORB_EFFECT_RADIUS;
    std::vector<std::uint32_t> deadFromOrb;

    for (auto it = m_medicOrbs.begin(); it != m_medicOrbs.end(); ) {
        auto& orb = it->second;

        sf::Vector2f velocity = orb.velocity * dt;

        sf::Vector2f nextPosX = orb.position + sf::Vector2f(velocity.x, 0.0f);
        if (m_map->checkCollision(nextPosX, Config::MEDIC_ORB_RADIUS)) {
            orb.velocity.x = -orb.velocity.x;
        } else {
            orb.position.x = nextPosX.x;
        }

        sf::Vector2f nextPosY = orb.position + sf::Vector2f(0.0f, velocity.y);
        if (m_map->checkCollision(nextPosY, Config::MEDIC_ORB_RADIUS)) {
            orb.velocity.y = -orb.velocity.y;
        } else {
            orb.position.y = nextPosY.y;
        }

        orb.tickAccumulator += dt;
        if (orb.tickAccumulator >= Config::MEDIC_ORB_TICK_INTERVAL) {
            orb.tickAccumulator = 0.0f;

            for (auto& [id, clientInfo] : m_clients) {
                float distSq = (clientInfo.position - orb.position).lengthSquared();
                if (distSq <= effectRadiusSq) {
                    float maxHp = HeroRegistry::getStats(clientInfo.pClass).maxHp;
                    clientInfo.hp += Config::MEDIC_ORB_HEAL_PER_TICK;
                    if (clientInfo.hp > maxHp) clientInfo.hp = maxHp;
                }
            }

            for (auto& [enemyId, enemy] : m_enemies) {
                float distSq = (enemy->getPosition() - orb.position).lengthSquared();
                if (distSq <= effectRadiusSq) {
                    enemy->takeDamage(Config::MEDIC_ORB_DAMAGE_PER_TICK);

                    if (m_clients.count(orb.ownerId)) {
                        sf::Packet damagePacket;
                        damagePacket << PacketType::PlayerDealtDamage << Config::MEDIC_ORB_DAMAGE_PER_TICK;
                        (void)m_socket.send(damagePacket, m_clients.at(orb.ownerId).ip, m_clients.at(orb.ownerId).port);
                    }

                    if (enemy->getHp() <= 0.0f) deadFromOrb.push_back(enemyId);
                }
            }
        }

        orb.lifetime -= dt;
        if (orb.lifetime <= 0.0f) {
            it = m_medicOrbs.erase(it);
        } else {
            ++it;
        }
    }

    for (auto enemyId : deadFromOrb) {
        if (m_enemies.count(enemyId)) {
            m_energyCells[m_globalEntityCounter++] = {m_enemies[enemyId]->getPosition(), 1, 0};
            m_enemies.erase(enemyId);
        }
    }
}

void ServerEngine::updateMedicBarriers(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();
    const float halfSpan = Config::MEDIC_BARRIER_SPAN / 2.0f;

    for (auto it = m_medicBarriers.begin(); it != m_medicBarriers.end(); ) {
        auto& barrier = it->second;

        for (auto& [enemyId, enemy] : m_enemies) {
            float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;
            if (!SectorMath::isInArcWall(
                    barrier.center,
                    barrier.facingAngle,
                    Config::MEDIC_BARRIER_ARC_RADIUS,
                    halfSpan,
                    Config::MEDIC_BARRIER_WALL_THICKNESS,
                    enemy->getPosition(),
                    enemyRadius)) {
                continue;
            }

            sf::Vector2f delta = enemy->getPosition() - barrier.center;
            float distSq = delta.lengthSquared();
            sf::Vector2f pushDir;
            if (distSq < 1e-4f) {
                pushDir = sf::Vector2f(std::cos(barrier.facingAngle), std::sin(barrier.facingAngle));
            } else {
                pushDir = delta / std::sqrt(distSq);
            }
            enemy->applyKnockback(pushDir, Config::MEDIC_BARRIER_KNOCKBACK);
        }

        barrier.lifetime -= dt;
        if (barrier.lifetime <= 0.0f) {
            it = m_medicBarriers.erase(it);
        } else {
            ++it;
        }
    }
}

MedicDroneData* ServerEngine::findDroneByOwner(std::uint32_t ownerId) {
    for (auto& [id, drone] : m_medicDrones) {
        if (drone.ownerId == ownerId) return &drone;
    }
    return nullptr;
}

std::uint32_t ServerEngine::findNearestEnemyId(const sf::Vector2f& from, float maxRange) const {
    const float maxRangeSq = maxRange * maxRange;
    std::uint32_t bestId = 0;
    float bestDistSq = maxRangeSq;

    for (const auto& [enemyId, enemy] : m_enemies) {
        float distSq = (enemy->getPosition() - from).lengthSquared();
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestId = enemyId;
        }
    }
    return bestId;
}

void ServerEngine::spawnDroneBlaster(std::uint32_t ownerId, sf::Vector2f startPos, sf::Vector2f targetPos) {
    const auto& stats = WeaponRegistry::getStats(WeaponType::DroneBlaster);

    sf::Vector2f dir = targetPos - startPos;
    float lenSq = dir.lengthSquared();
    if (lenSq < 1e-4f) {
        dir = sf::Vector2f(1.0f, 0.0f);
    } else {
        dir /= std::sqrt(lenSq);
    }

    m_serverProjectiles.push_back({
        ownerId,
        startPos,
        dir * stats.speed,
        WeaponType::DroneBlaster,
        stats.lifetime,
        stats.radius
    });

    sf::Packet shootPacket;
    shootPacket << PacketType::DroneShoots << startPos << targetPos;
    for (const auto& [id, clientInfo] : m_clients) {
        (void)m_socket.send(shootPacket, clientInfo.ip, clientInfo.port);
    }
}

void ServerEngine::handleMedicDroneCommand(std::uint32_t playerId, sf::Vector2f targetPos) {
    MedicDroneData* drone = findDroneByOwner(playerId);

    if (!drone) {
        if (!m_clients.count(playerId)) return;

        std::uint32_t droneId = m_globalEntityCounter++;
        sf::Vector2f spawnPos = m_clients.at(playerId).position;
        m_medicDrones[droneId] = {
            spawnPos,
            Config::MEDIC_DRONE_LIFETIME,
            playerId,
            MedicDroneState::Orbit,
            playerId,
            targetPos,
            0.0f,
            0.0f,
            0.0f,
            false
        };
        drone = &m_medicDrones[droneId];

        sf::Packet spawnPacket;
        spawnPacket << PacketType::SpawnMedicDrone << droneId << playerId << spawnPos;
        for (const auto& [id, clientInfo] : m_clients) {
            (void)m_socket.send(spawnPacket, clientInfo.ip, clientInfo.port);
        }
    }

    const float detectSq = Config::MEDIC_DRONE_PLAYER_DETECT_RADIUS * Config::MEDIC_DRONE_PLAYER_DETECT_RADIUS;
    std::uint32_t nearestPlayerId = 0;
    float nearestDistSq = detectSq;

    for (const auto& [id, clientInfo] : m_clients) {
        float distSq = (clientInfo.position - targetPos).lengthSquared();
        if (distSq <= nearestDistSq) {
            nearestDistSq = distSq;
            nearestPlayerId = id;
        }
    }

    if (nearestPlayerId != 0) {
        drone->state = MedicDroneState::Orbit;
        drone->orbitTargetId = nearestPlayerId;
        drone->atSentry = false;
    } else {
        drone->state = MedicDroneState::Sentry;
        drone->sentryPos = targetPos;
        drone->atSentry = false;
    }
}

void ServerEngine::updateMedicDrones(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();

    for (auto it = m_medicDrones.begin(); it != m_medicDrones.end(); ) {
        auto& drone = it->second;

        if (!m_clients.count(drone.ownerId)) {
            it = m_medicDrones.erase(it);
            continue;
        }

        if (drone.state == MedicDroneState::Orbit) {
            if (!m_clients.count(drone.orbitTargetId)) {
                drone.orbitTargetId = drone.ownerId;
            }

            sf::Vector2f targetPos = m_clients.at(drone.orbitTargetId).position;
            drone.orbitAngle += Config::MEDIC_DRONE_ORBIT_SPEED * dt;
            drone.position = targetPos + sf::Vector2f(
                std::cos(drone.orbitAngle) * Config::MEDIC_DRONE_ORBIT_RADIUS,
                std::sin(drone.orbitAngle) * Config::MEDIC_DRONE_ORBIT_RADIUS
            );

            drone.healTimer -= dt;
            if (drone.healTimer <= 0.0f) {
                drone.healTimer = Config::MEDIC_DRONE_SYM_HEAL_INTERVAL;
                if (m_clients.count(drone.orbitTargetId)) {
                    auto& target = m_clients.at(drone.orbitTargetId);
                    float maxHp = HeroRegistry::getStats(target.pClass).maxHp;
                    if (target.hp < maxHp) {
                        target.hp += Config::MEDIC_DRONE_SYM_HEAL_AMOUNT;
                        if (target.hp > maxHp) target.hp = maxHp;
                    }
                }
            }
        } else {
            sf::Vector2f toSentry = drone.sentryPos - drone.position;
            float distSq = toSentry.lengthSquared();
            if (!drone.atSentry && distSq > Config::MEDIC_DRONE_SENTRY_ARRIVE_DIST * Config::MEDIC_DRONE_SENTRY_ARRIVE_DIST) {
                float dist = std::sqrt(distSq);
                sf::Vector2f step = (toSentry / dist) * Config::MEDIC_DRONE_MOVE_SPEED * dt;
                if (step.lengthSquared() >= distSq) {
                    drone.position = drone.sentryPos;
                    drone.atSentry = true;
                } else {
                    drone.position += step;
                }
            } else {
                drone.atSentry = true;
                drone.position = drone.sentryPos;
            }
        }

        drone.shootTimer -= dt;
        if (drone.shootTimer <= 0.0f) {
            float interval = drone.state == MedicDroneState::Orbit
                ? Config::MEDIC_DRONE_SYM_SHOOT_INTERVAL
                : Config::MEDIC_DRONE_SENTRY_SHOOT_INTERVAL;
            drone.shootTimer = interval;

            std::uint32_t enemyId = findNearestEnemyId(drone.position, Config::MEDIC_DRONE_ATTACK_RANGE);
            if (enemyId != 0 && m_enemies.count(enemyId)) {
                spawnDroneBlaster(drone.ownerId, drone.position, m_enemies[enemyId]->getPosition());
            }
        }

        drone.lifetime -= dt;
        if (drone.lifetime <= 0.0f) {
            it = m_medicDrones.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerEngine::spawnMedicNeedle(std::uint32_t ownerId, sf::Vector2f startPos, sf::Vector2f targetPos) {
    const auto& stats = WeaponRegistry::getStats(WeaponType::MedicNeedle);

    sf::Vector2f dir = targetPos - startPos;
    float lenSq = dir.lengthSquared();
    if (lenSq < 1e-4f) {
        dir = sf::Vector2f(1.0f, 0.0f);
    } else {
        dir /= std::sqrt(lenSq);
    }

    m_serverProjectiles.push_back({
        ownerId,
        startPos,
        dir * stats.speed,
        WeaponType::MedicNeedle,
        stats.lifetime,
        stats.radius
    });
}

void ServerEngine::updateServerProjectiles(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();
    const float needleDamage = WeaponRegistry::getStats(WeaponType::MedicNeedle).damage;
    const float blasterDamage = WeaponRegistry::getStats(WeaponType::DroneBlaster).damage;
    std::vector<std::uint32_t> deadFromProjectile;

    for (auto it = m_serverProjectiles.begin(); it != m_serverProjectiles.end(); ) {
        auto& proj = *it;

        if (proj.weapon != WeaponType::MedicNeedle && proj.weapon != WeaponType::DroneBlaster) {
            ++it;
            continue;
        }

        proj.position += proj.velocity * dt;
        proj.lifetime -= dt;

        bool destroyed = false;

        if (m_map->checkCollision(proj.position, proj.radius)) {
            destroyed = true;
        }

        if (!destroyed && proj.weapon == WeaponType::MedicNeedle) {
            for (auto& [playerId, clientInfo] : m_clients) {
                if (playerId == proj.ownerId) continue;

                float heroRadius = HeroRegistry::getStats(clientInfo.pClass).radius;
                sf::Vector2f diff = proj.position - clientInfo.position;
                float collDist = heroRadius + proj.radius;

                if (diff.lengthSquared() < collDist * collDist) {
                    float maxHp = HeroRegistry::getStats(clientInfo.pClass).maxHp;
                    clientInfo.hp += Config::MEDIC_NEEDLE_HEAL;
                    if (clientInfo.hp > maxHp) clientInfo.hp = maxHp;
                    destroyed = true;
                    break;
                }
            }
        }

        if (!destroyed) {
            for (auto& [enemyId, enemy] : m_enemies) {
                float enemyRadius = EnemyRegistry::getStats(enemy->getType()).radius;
                sf::Vector2f diff = proj.position - enemy->getPosition();
                float collDist = enemyRadius + proj.radius;

                if (diff.lengthSquared() >= collDist * collDist) continue;

                if (proj.weapon == WeaponType::DroneBlaster) {
                    if (std::find(proj.hitEnemies.begin(), proj.hitEnemies.end(), enemyId) != proj.hitEnemies.end()) {
                        continue;
                    }
                    proj.hitEnemies.push_back(enemyId);

                    enemy->takeDamage(blasterDamage);

                    if (m_clients.count(proj.ownerId)) {
                        sf::Packet damagePacket;
                        damagePacket << PacketType::PlayerDealtDamage << blasterDamage;
                        (void)m_socket.send(damagePacket, m_clients.at(proj.ownerId).ip, m_clients.at(proj.ownerId).port);
                    }

                    if (enemy->getHp() <= 0.0f) deadFromProjectile.push_back(enemyId);
                    continue;
                }

                enemy->takeDamage(needleDamage);
                enemy->applyPoison(Config::MEDIC_NEEDLE_POISON_DURATION, Config::MEDIC_NEEDLE_POISON_DPS);

                if (m_clients.count(proj.ownerId)) {
                    sf::Packet damagePacket;
                    damagePacket << PacketType::PlayerDealtDamage << needleDamage;
                    (void)m_socket.send(damagePacket, m_clients.at(proj.ownerId).ip, m_clients.at(proj.ownerId).port);
                }

                if (enemy->getHp() <= 0.0f) deadFromProjectile.push_back(enemyId);
                destroyed = true;
                break;
            }
        }

        if (destroyed || proj.lifetime <= 0.0f) {
            it = m_serverProjectiles.erase(it);
        } else {
            ++it;
        }
    }

    for (std::uint32_t enemyId : deadFromProjectile) {
        if (m_enemies.count(enemyId)) {
            m_energyCells[m_globalEntityCounter++] = {m_enemies[enemyId]->getPosition(), 1, 0};
            m_enemies.erase(enemyId);
        }
    }
}

void ServerEngine::sendWorldState(){
    if(m_clients.empty()) return;

    sf::Packet worldPacket;

    // Players info
    worldPacket << PacketType::WorldState << static_cast<std::uint32_t>(m_clients.size());
    
    for(const auto& [clientId, info] : m_clients){
        worldPacket << clientId << info.pClass << info.position << info.hp << info.stealthTimer;
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

    worldPacket << static_cast<std::uint32_t>(m_decoys.size());
    for(const auto& [decoyId, decoy] : m_decoys){
        worldPacket << decoyId << decoy.pos << decoy.hp << Config::VANGUARD_DECOY_HP;
    }

    worldPacket << static_cast<std::uint32_t>(m_medicOrbs.size());
    for(const auto& [orbId, orb] : m_medicOrbs){
        worldPacket << orbId << orb.position;
    }

    worldPacket << static_cast<std::uint32_t>(m_medicDrones.size());
    for(const auto& [droneId, drone] : m_medicDrones){
        worldPacket << droneId << drone.ownerId << drone.position << drone.state << drone.lifetime;
    }

    // Sending
    for(const auto& [clientId, info] : m_clients){
        (void)m_socket.send(worldPacket, info.ip, info.port);
    }
}
