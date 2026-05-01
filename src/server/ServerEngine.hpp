#pragma once

#include "NetworkProtocol.hpp"

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
};