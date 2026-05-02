#pragma once

#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
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
        float hp = Config::PLAYER_MAX_HP;
    };

    std::map<std::uint32_t, ClientInfo> m_clients;

    struct EnemyInfo {
        sf::Vector2f position;
        float speed;
        float hp = Config::ENEMY_MAX_HP;
        sf::Clock lastAttackTime;
    };

    std::map<std::uint32_t, EnemyInfo> m_enemies;
    sf::Clock m_enemySpawnTimer;

    std::uint32_t m_globalEntityCounter = 1;
};