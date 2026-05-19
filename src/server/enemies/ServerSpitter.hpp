#pragma once

#include "ServerEnemy.hpp"

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
        const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies
    ) override;

};