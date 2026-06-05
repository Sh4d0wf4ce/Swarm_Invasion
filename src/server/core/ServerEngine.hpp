#pragma once

#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include "WeaponRegistry.hpp"
#include "ServerTypes.hpp"
#include "AIDirector.hpp"
#include "Config.hpp"

#include <SFML/System.hpp>
#include <memory>
#include <random>
#include <map>

class ServerEngine{
public:
    ServerEngine();
    void run();

private:
    void processNetwork();
    void update(sf::Time deltaTime);

    void handlePing(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port);
    void handlePlayerPosition(sf::Packet& packet);
    void handleEntityHit(sf::Packet& packet);
    void handleJoinRequest(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port);
    void handlePlayerShoots(sf::Packet& packet);
    void handlePlayerDisconnect(sf::Packet& packet);
    void handleCardSelected(sf::Packet& packet);
    void handleAbilityHit(sf::Packet& packet);
    void handleAbilityUsed(sf::Packet& packet);

    void proccessUpgradeMenuTimeout();
    void removeAFKPlayers();
    void updateEnergyCells(sf::Time deltaTime);
    void updateHealingFields(sf::Time deltaTime);
    void updateBlackHoles(sf::Time deltaTime);
    void explodeDecoy(std::uint32_t decoyId, const DecoyData& decoy);
    bool playerHasActiveDecoy(std::uint32_t playerId) const;
    void sendWorldState();

    std::shared_ptr<MapGenerator> m_map;

    AIDirector m_aiDirector;

    sf::UdpSocket m_socket;
    sf::Clock m_clock;
    sf::Time m_timePerTick;
    bool m_isRunning;

    int  m_tickCounter;

    std::map<std::uint32_t, ClientInfo> m_clients;
    std::map<std::uint32_t, EnergyCellInfo> m_energyCells;
    std::map<std::uint32_t, HealFieldInfo> m_healFields;
    std::map<std::uint32_t, BlackHoleData> m_blackHoles;
    std::map<std::uint32_t, DecoyData> m_decoys;

    std::map<std::uint32_t, std::unique_ptr<ServerEnemy>> m_enemies;

    std::uint32_t m_globalEntityCounter = 1;

    int m_teamLevel = 1;
    int m_teamExp = 0;
    int m_teamExpMax = 10;

    bool m_isPaused = false;
    sf::Clock m_upgradeTimer;
    std::map<std::uint32_t, int> m_playerChoices;
};