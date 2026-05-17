#pragma once

#include "NetworkProtocol.hpp"

#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>

struct WeaponStats{
    float speed;
    float radius;
    float damage;
    float lifetime;
    sf::Color color;
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
        };

        for(auto& [key, val] : json.items()){
           if(typeMap.find(key) != typeMap.end()){
                WeaponType type = typeMap[key];
                WeaponStats stats;

                stats.speed = val["speed"];
                stats.radius = val["radius"];
                stats.damage = val["damage"];
                stats.lifetime = val["lifetime"];

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