#pragma once

#include <SFML/System.hpp>
#include "NetworkProtocol.hpp"

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
};