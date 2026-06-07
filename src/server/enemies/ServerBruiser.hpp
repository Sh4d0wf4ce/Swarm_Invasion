#pragma once 

#include "ServerEnemy.hpp"

enum class BruiserState {
    Chasing,
    Preparing,
    Charging,
    Resting
};

class ServerBruiser : public ServerEnemy {
public:
    using ServerEnemy::ServerEnemy;

    std::vector<std::uint32_t> update(
        sf::Time deltaTime,
        std::map<std::uint32_t, 
        ClientInfo>& clients, 
        std::shared_ptr<MapGenerator> map, 
        const std::vector<std::vector<int>>& flowField, 
        std::vector<EnemyShootEvent>& outShootEvents, 
        const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies,
        const SpatialGrid& grid
    ) override;

private:
    // Charge State Machine
    BruiserState m_state{BruiserState::Chasing};
    sf::Vector2f m_chargeDirection;
    sf::Clock m_stateTimer;
    sf::Clock m_cooldownTimer;
};
