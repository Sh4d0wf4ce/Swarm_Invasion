#pragma once

#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include "NetworkProtocol.hpp"
#include <vector>

// ==========================================
// Pickups & Ability Fields
// ==========================================

/**
 * @brief Experience pickup that magnetizes toward a nearby player.
 */
struct EnergyCellInfo {
    sf::Vector2f position;
    int expValue;
    std::uint32_t targetPlayerId{0};
};

/**
 * @brief Temporary area that heals players standing inside its radius.
 */
struct HealFieldInfo{
    sf::Vector2f position;
    float radius{100.0f};
    float duration{5.0f};
    float healPerSecond{10.0f};
};

/**
 * @brief Juggernaut black hole that pulls and damages enemies over time.
 */
struct BlackHoleData{
    sf::Vector2f position;
    float duration;
    std::uint32_t ownerId;
};

// ==========================================
// Vanguard & Medic World Objects
// ==========================================

/**
 * @brief Vanguard decoy entity that mimics a player target for enemy AI.
 */
struct DecoyData{
    sf::Vector2f pos;
    float hp;
    std::uint32_t ownerId;
    float lifetime;
};

/**
 * @brief Medic orb that bounces off walls and applies periodic heal/damage ticks.
 */
struct MedicOrbData {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float tickAccumulator{0.0f};
    std::uint32_t ownerId;
};

/**
 * @brief Medic arc barrier that knocks back enemies intersecting its wall segment.
 */
struct MedicBarrierData {
    sf::Vector2f center;
    float facingAngle;
    float lifetime;
    std::uint32_t ownerId;
};

/**
 * @brief Medic support drone that orbits allies or holds a sentry position.
 */
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

// ==========================================
// Server Projectiles
// ==========================================

/**
 * @brief Authoritative server-side projectile used for medic needle and drone blaster simulation.
 */
struct ServerProjectileData {
    std::uint32_t ownerId;
    sf::Vector2f position;
    sf::Vector2f velocity;
    WeaponType weapon;
    float lifetime;
    float radius;
    std::vector<std::uint32_t> hitEnemies;
};

// ==========================================
// Connected Player State
// ==========================================

/**
 * @brief Runtime state for a connected client tracked by the server.
 */
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
    float hpMultiplier{1.0f};
    float speedMultiplier{1.0f};
    float damageMultiplier{1.0f};
    float cooldownMultiplier{1.0f};
    std::vector<std::string> ownedAugments;
};
