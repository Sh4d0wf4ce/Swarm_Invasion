#pragma once

#include "NetworkProtocol.hpp"

#include <SFML/Graphics/Color.hpp>
#include <unordered_map>

struct HeroStats{
    float maxHp;
    float speed;
    float radius;
    sf::Color color;
};

class HeroRegistry{
public:
    static const HeroStats& getStats(PlayerClass pClass){
        static const std::unordered_map<PlayerClass, HeroStats> stats = {
            {PlayerClass::Soldier, {100.0f, 300.0f, 20.0f, sf::Color::White}},
            {PlayerClass::Scout,   {75.0f,  450.0f, 15.0f, sf::Color::Yellow}},
            {PlayerClass::Tank,    {200.0f, 150.0f, 30.0f, sf::Color::Blue}},
            {PlayerClass::Medic,   {100.0f, 320.0f, 20.0f, sf::Color::Green}},
        };

        return stats.at(pClass);
    }
};