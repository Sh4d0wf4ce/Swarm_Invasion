#pragma once
#include "NetworkProtocol.hpp"
#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>


/**
 * @brief Combat and presentation stats for an enemy archetype.
 */
struct EnemyStats{
    float maxHp;
    float speed;
    float radius;
    float damage;
    float attackCooldown;
    sf::Color color;
};

/**
 * @brief Central registry that loads and exposes enemy tuning data from JSON files.
 */
class EnemyRegistry {
public:
    /**
     * @brief Loads enemy configuration from a JSON file and populates the registry.
     * @param filepath Path to the enemies JSON configuration file.
     */
    static void loadConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[REGISTRY ERROR] Could not open " << filepath << "!\n";
            return;
        }

        nlohmann::json j;
        file >> j;
        file.close();
        std::unordered_map<std::string, EnemyType> typeMap = {
            {"Crawler", EnemyType::Crawler},
            {"Bruiser", EnemyType::Bruiser},
            {"Spitter", EnemyType::Spitter},
            {"Kamikaze", EnemyType::Kamikaze}
        };
        for (auto& [key, val] : j.items()) {
            if (typeMap.find(key) != typeMap.end()) {
                EnemyType type = typeMap[key];
                EnemyStats stats;
                stats.maxHp = val["maxHp"];
                stats.speed = val["speed"];
                stats.radius = val["radius"];
                stats.damage = val["damage"];
                stats.attackCooldown = val["attackCooldown"];
                auto c = val["color"];
                stats.color = sf::Color(c[0], c[1], c[2], c[3]);
                m_stats[type] = stats;
            }
        }
        std::cout << "[REGISTRY] Successfully loaded " << filepath << " (" << m_stats.size() << " enemies)\n";
    }

    /**
     * @brief Returns stats for a given enemy type.
     * @param eType Enemy type identifier.
     * @return Reference to the enemy stats for @p eType.
     * @throws std::out_of_range if @p eType was not loaded.
     */
    static const EnemyStats& getStats(EnemyType eType) {
        return m_stats.at(eType);
    }

private:
    static inline std::unordered_map<EnemyType, EnemyStats> m_stats;
};
