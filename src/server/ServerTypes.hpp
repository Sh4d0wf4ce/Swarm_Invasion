#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include "NetworkProtocol.hpp"

struct EnergyCellInfo {
    sf::Vector2f position;
    int expValue;
    std::uint32_t targetPlayerId = 0;
};


struct ClientInfo {
    sf::IpAddress ip;
    unsigned short port;
    sf::Vector2f position;
    sf::Clock lastActivity;
    float hp;
    float speed;
    PlayerClass pClass;
};