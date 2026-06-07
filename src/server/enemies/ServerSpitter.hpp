#pragma once

#include "ServerEnemy.hpp"

/**
 * @brief Ranged enemy that fires acid spit at distance and falls back to melee when blocked.
 */
class ServerSpitter : public ServerEnemy {
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

};
