#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include "NetworkProtocol.hpp"

struct EnergyCellInfo {
    sf::Vector2f position;
    int expValue;
    std::uint32_t targetPlayerId{0};
};

struct HealFieldInfo{
    sf::Vector2f position;
    float radius{100.0f};
    float duration{5.0f};
    float healPerSecond{10.0f};
};

struct BlackHoleData{
    sf::Vector2f position;
    float duration;
    std::uint32_t ownerId;
};


struct ClientInfo {
    sf::IpAddress ip;
    unsigned short port;
    sf::Vector2f position;
    sf::Clock lastActivity;
    float hp;
    float speed;
    PlayerClass pClass;
    float invTimer{0.0f};
};