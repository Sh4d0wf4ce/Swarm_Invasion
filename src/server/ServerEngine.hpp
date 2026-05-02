#pragma once

#include "NetworkProtocol.hpp"
#include "Config.hpp"

#include <SFML/System.hpp>
#include <map>

class ServerEngine{
public:
    ServerEngine();
    void run();

private:
    void processNetwork();
    void update(sf::Time deltaTime);

    sf::UdpSocket m_socket;
    sf::Clock m_clock;
    sf::Time m_timePerTick;
    bool m_isRunning;

    int  m_tickCounter;

    struct ClientInfo {
        sf::IpAddress ip;
        unsigned short port;
        sf::Vector2f position;
        sf::Clock lastActivity;
    };

    std::map<std::uint32_t, ClientInfo> m_clients;
    std::uint32_t m_nextPlayerId = 1;

    struct EnemyInfo {
        sf::Vector2f position;
        float speed;
    };

    std::map<std::uint32_t, EnemyInfo> m_enemies;
    std::uint32_t m_nextEnemyId = 100000;
    sf::Clock m_enemySpawnTimer;
};