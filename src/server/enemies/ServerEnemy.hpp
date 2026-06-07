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

// ==========================================
// Combat Events & Status Effects
// ==========================================
/**
 * @brief Active poison damage-over-time effect applied to a server enemy.
 */
struct PoisonEffect {
    float remainingTime;
    float damagePerSecond;
};

/**
 * @brief Ranged attack event emitted by an enemy for client-side projectile relay.
 */
struct EnemyShootEvent{
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    WeaponType weapon;
};


/**
 * @brief Abstract authoritative server enemy with shared movement, combat, and status helpers.
 *
 * Derived classes implement update() with type-specific behaviour while reusing flow-field
 * chase, separation, line-of-sight checks, knockback, and poison handling.
 */
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

    void setPosition(sf::Vector2f newPos) { m_position = newPos; }
    void takeDamage(float damage) { m_hp -= damage; }

    // Status Effects & Knockback
    void applyPoison(float duration, float damagePerSecond);
    void tickStatusEffects(float dt);
    void applyKnockback(sf::Vector2f direction, float force);

protected:
    // Perception & Movement Helpers
	bool hasLineOfSight(sf::Vector2f start, sf::Vector2f end, float radius, std::shared_ptr<MapGenerator> map);
    sf::Vector2f calculateSeparation(const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, float myRadius, const SpatialGrid& grid);
    std::vector<std::uint32_t> performMeleeChase(sf::Time deltaTime, std::map<std::uint32_t, ClientInfo>& clients, std::shared_ptr<MapGenerator> map, const std::vector<std::vector<int>>& flowField, const std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& allEnemies, const SpatialGrid& grid);
    std::uint32_t getClosestPlayerId(const std::map<std::uint32_t, ClientInfo>& clients, float& outMinDistSq) const;


    // Core State
    std::uint32_t m_id;
    sf::Vector2f m_position;
    float m_hp;
    float m_speed;
    EnemyType m_type;
    sf::Clock m_lastAttackTime;

    sf::Vector2f m_knockbackVelocity{0.0f, 0.0f};
    std::vector<PoisonEffect> m_poisonEffects;
};
