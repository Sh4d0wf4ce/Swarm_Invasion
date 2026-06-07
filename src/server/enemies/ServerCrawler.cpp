#include "ServerCrawler.hpp"

/**
 * @brief Delegates entirely to shared melee chase behaviour.
 * @param deltaTime Elapsed time since the last update.
 * @param clients Mutable player targets for damage and pursuit.
 * @param map Map used for collision and pathfinding.
 * @param flowField Precomputed BFS cost field toward players.
 * @param outShootEvents Unused; crawlers do not shoot.
 * @param allEnemies All enemies used for separation calculations.
 * @param grid Spatial grid for neighbour queries.
 * @return Player IDs killed by melee contact this tick.
 */
std::vector<std::uint32_t> ServerCrawler::update(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, std::vector<EnemyShootEvent>& outShootEvents, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid) {
    return performMeleeChase(deltaTime, clients, map, flowField, allEnemies, grid);
}
