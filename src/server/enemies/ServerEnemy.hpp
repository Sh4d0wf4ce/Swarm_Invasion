#pragma once

#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "../core/ServerTypes.hpp"
#include "EnemyRegistry.hpp"
#include "../core/SpatialGrid.hpp"

#include <SFML/System.hpp>
#include <vector>
#include <memory>
#include <map>

struct EnemyShootEvent{
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    WeaponType weapon;
};

class ServerEnemy{
public:
    ServerEnemy(std::uint32_t id, const sf::Vector2f& startPos, EnemyType type);
    virtual ~ServerEnemy() = default;

    virtual std::vector<std::uint32_t> update(
        sf::Time deltaTime,
        std::map<std::uint32_t, ClientInfo>& clients,
        std::shared_ptr<MapGenerator> map,
        const std::vector<std::vector<int>>& flowField,
        std::vector<EnemyShootEvent>& outShootEvents,
        const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies,
        const SpatialGrid& grid
    ) = 0;

    std::uint32_t getId() const { return m_id; }
    sf::Vector2f getPosition() const { return m_position; }
    EnemyType getType() const { return m_type; }
    float getHp() const { return m_hp; }

    void takeDamage(float damage) { m_hp -= damage; }

protected:
	bool hasLineOfSight(sf::Vector2f start, sf::Vector2f end, float radius, std::shared_ptr<MapGenerator> map);
    sf::Vector2f calculateSeparation(const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, float myRadius, const SpatialGrid& grid);

    std::vector<std::uint32_t> performMeleeChase(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid);
    std::uint32_t getClosestPlayerId(const std::map<std::uint32_t, ClientInfo>& clients, float& outMinDistSq) const;

    std::uint32_t m_id;
    sf::Vector2f m_position;
    float m_hp;
    float m_speed;
    EnemyType m_type;
    sf::Clock m_lastAttackTime;
};