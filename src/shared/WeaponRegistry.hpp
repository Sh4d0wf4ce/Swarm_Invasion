#pragma once
#include "NetworkProtocol.hpp"
#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>


// Weapon Stat Definition
struct WeaponStats{
    float speed;
    float radius;
    float damage;
    float lifetime;
    sf::Color color;
    int pellets;
    float spreadAngle;
    float explosionRadius;
};

class WeaponRegistry{
public:
    static void loadConfig(const std::string& filepath){
        std::ifstream file(filepath);
        if(!file.is_open()){
            std::cerr << "[REGISTRY ERROR] Could not open " << filepath << "!\n";
            return;
        }

        nlohmann::json json;
        file >> json;
        file.close();
        
        std::unordered_map<std::string, WeaponType> typeMap = {
            {"Rifle", WeaponType::Rifle},
            {"Laser", WeaponType::Laser},
            {"Rocket", WeaponType::Rocket},
            {"AcidSpit", WeaponType::AcidSpit},
            {"Shotgun", WeaponType::Shotgun},
            {"Shuriken", WeaponType::Shuriken},
            {"VanguardWave", WeaponType::VanguardWave},
            {"MedicNeedle", WeaponType::MedicNeedle},
            {"DroneBlaster", WeaponType::DroneBlaster},
        };

        for(auto& [key, val] : json.items()){
           if(typeMap.find(key) != typeMap.end()){
                WeaponType type = typeMap[key];
                WeaponStats stats;
                stats.speed = val["speed"];
                stats.radius = val["radius"];
                stats.damage = val["damage"];
                stats.lifetime = val["lifetime"];
                stats.pellets = val.value("pellets", 1);
                stats.spreadAngle = val.value("spreadAngle", 0.0f);
                stats.explosionRadius = val.value("explosionRadius", 0.0f);
                auto c = val["color"];
                stats.color = sf::Color(c[0], c[1], c[2], c[3]);
                m_stats[type] = stats;
           } 
        }
        std::cout << "[REGISTRY] Successfully loaded " << filepath << " (" << m_stats.size() << " weapons)\n";
    }
    static const WeaponStats& getStats(WeaponType wType){
        return m_stats.at(wType);
    }

private:
    static inline std::unordered_map<WeaponType, WeaponStats> m_stats;
};
