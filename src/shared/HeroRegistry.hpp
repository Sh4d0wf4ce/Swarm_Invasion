#pragma once

#include "NetworkProtocol.hpp"

#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>

struct HeroStats{
    float maxHp;
    float speed;
    float radius;
    sf::Color color;
    WeaponType defaultWeapon;
};

class HeroRegistry {
public:
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
            {"Scout", PlayerClass::Scout},
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

    static const HeroStats& getStats(PlayerClass pClass) {
        return m_stats.at(pClass);
    }

private:
    static inline std::unordered_map<PlayerClass, HeroStats> m_stats;
};