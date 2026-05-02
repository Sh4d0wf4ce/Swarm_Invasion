#pragma once

#include "NetworkProtocol.hpp"

#include <SFML/Graphics/Color.hpp>
#include <unordered_map>

struct WeaponStats{
    float speed;
    float radius;
    float damage;
    float lifetime;
    sf::Color color;
};

class WeaponRegistry{
public:
    static const WeaponStats& getStats(WeaponType wType){
        static const std::unordered_map<WeaponType, WeaponStats> stats = {
            {WeaponType::Rifle,  {800.0f,  5.0f,  25.0f,  3.0f, sf::Color::Cyan}},
            {WeaponType::Laser,  {1500.0f, 3.0f,  10.0f,  1.5f, sf::Color::Yellow}},
            {WeaponType::Rocket, {300.0f,  12.0f, 75.0f,  5.0f, sf::Color::Red}}
        };

        return stats.at(wType);
    }
};