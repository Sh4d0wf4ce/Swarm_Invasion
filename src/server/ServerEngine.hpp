#pragma once

#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include "WeaponRegistry.hpp"
#include "Config.hpp"

#include <SFML/System.hpp>
#include <memory>
#include <map>

class ServerEngine{
public:
    ServerEngine();
    void run();

private:
    void processNetwork();
    void update(sf::Time deltaTime);

    bool checkCollision(const sf::Vector2f& pos, float radius);

    std::shared_ptr<MapGenerator> m_map;

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
        float hp;
        float speed;
        PlayerClass pClass;
    };

    std::map<std::uint32_t, ClientInfo> m_clients;

    struct EnemyInfo {
        sf::Vector2f position;
        float speed;
        float hp;
        sf::Clock lastAttackTime;
        EnemyType type;
    };

    std::map<std::uint32_t, EnemyInfo> m_enemies;
    sf::Clock m_enemySpawnTimer;

    std::uint32_t m_globalEntityCounter = 1;
};