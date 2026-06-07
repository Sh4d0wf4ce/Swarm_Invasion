#include "ServerSpitter.hpp"

std::vector<std::uint32_t> ServerSpitter::update(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, std::vector<EnemyShootEvent>& outShootEvents, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid) {
    std::vector<std::uint32_t> deadPlayers;
    const auto& eStats = EnemyRegistry::getStats(m_type);
    float shootRange = 350.0f;
    float minDistanceSq;
    std::uint32_t targetId = getClosestPlayerId(clients, minDistanceSq);
    if (minDistanceSq > shootRange * shootRange || !hasLineOfSight(m_position, clients.at(targetId).position, eStats.radius, map)) {
        return performMeleeChase(deltaTime, clients, map, flowField, allEnemies, grid);
    }

    // --- Fire acid spit when in range with line of sight ---
    if (m_lastAttackTime.getElapsedTime().asSeconds() > eStats.attackCooldown) {
        outShootEvents.push_back({m_position, clients.at(targetId).position, WeaponType::AcidSpit});
        m_lastAttackTime.restart();
    }

    // --- Maintain spacing while holding firing position ---
    sf::Vector2f separationVector = calculateSeparation(allEnemies, eStats.radius, grid);
    if (separationVector.lengthSquared() > 0) {
        sf::Vector2f velocity = separationVector * m_speed * 0.5f * deltaTime.asSeconds();
        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        if(!map->checkCollision(nextPosX, eStats.radius)) m_position.x = nextPosX.x;
        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        if(!map->checkCollision(nextPosY, eStats.radius)) m_position.y = nextPosY.y;
    }
    return deadPlayers;
}
