#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include "NetworkProtocol.hpp"
#include <vector>

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

struct DecoyData{
    sf::Vector2f pos;
    float hp;
    std::uint32_t ownerId;
    float lifetime;
};

struct MedicOrbData {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float tickAccumulator{0.0f};
    std::uint32_t ownerId;
};

struct ServerProjectileData {
    std::uint32_t ownerId;
    sf::Vector2f position;
    sf::Vector2f velocity;
    WeaponType weapon;
    float lifetime;
    float radius;
    std::vector<std::uint32_t> hitEnemies;
};

struct MedicBarrierData {
    sf::Vector2f center;
    float facingAngle;
    float lifetime;
    std::uint32_t ownerId;
};

struct MedicDroneData {
    sf::Vector2f position;
    float lifetime;
    std::uint32_t ownerId;
    MedicDroneState state{MedicDroneState::Orbit};
    std::uint32_t orbitTargetId{0};
    sf::Vector2f sentryPos;
    float orbitAngle{0.0f};
    float healTimer{0.0f};
    float shootTimer{0.0f};
    bool atSentry{false};
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
    float stealthTimer{0.0f};
};