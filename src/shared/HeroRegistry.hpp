#pragma once
#include "NetworkProtocol.hpp"
#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>

/**
 * @brief Base combat and presentation stats for a playable hero class.
 */
struct HeroStats{
    float maxHp;
    float speed;
    float radius;
    sf::Color color;
    WeaponType defaultWeapon;
};

/**
 * @brief Central registry that loads and exposes hero class tuning data from JSON files.
 */
class HeroRegistry {
public:
    /**
     * @brief Loads hero configuration from a JSON file and populates the registry.
     * @param filepath Path to the heroes JSON configuration file.
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
        std::unordered_map<std::string, PlayerClass> classMap = {
            {"Soldier", PlayerClass::Soldier},
            {"Medic", PlayerClass::Medic},
            {"Juggernaut", PlayerClass::Juggernaut},
            {"Vanguard", PlayerClass::Vanguard}
        };
        std::unordered_map<std::string, WeaponType> weaponMap = {
            {"Rifle", WeaponType::Rifle},
            {"Laser", WeaponType::Laser},
            {"Rocket", WeaponType::Rocket},
            {"AcidSpit", WeaponType::AcidSpit},
            {"Shotgun", WeaponType::Shotgun}
        };
        for (auto& [key, val] : j.items()) {
            if (classMap.find(key) != classMap.end()) {
                PlayerClass pClass = classMap[key];
                HeroStats stats;
                stats.maxHp = val["maxHp"];
                stats.speed = val["speed"];
                stats.radius = val["radius"];
                std::string wepStr = val["defaultWeapon"];
                stats.defaultWeapon = weaponMap.count(wepStr) ? weaponMap[wepStr] : WeaponType::Rifle;
                auto c = val["color"];
                stats.color = sf::Color(c[0], c[1], c[2], c[3]);
                m_stats[pClass] = stats;
            }
        }
        std::cout << "[REGISTRY] Successfully loaded " << filepath << " (" << m_stats.size() << " heroes)\n";
    }

    /**
     * @brief Returns stats for a given hero class.
     * @param pClass Player class identifier.
     * @return Reference to the hero stats for @p pClass.
     * @throws std::out_of_range if @p pClass was not loaded.
     */
    static const HeroStats& getStats(PlayerClass pClass) {
        return m_stats.at(pClass);
    }

private:
    static inline std::unordered_map<PlayerClass, HeroStats> m_stats;
};
