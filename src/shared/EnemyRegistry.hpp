#pragma once

#include "NetworkProtocol.hpp"

#include <SFML/Graphics/Color.hpp>
#include <unordered_map>

struct EnemyStats{
    float maxHp;
    float speed;
    float radius;
    float damage;
    float attackCooldown;
    sf::Color color;
};

class EnemyRegistry{
public:
    static const EnemyStats& getStats(EnemyType eType){
        static const std::unordered_map<EnemyType, EnemyStats> stats = {
            {EnemyType::Crawler, {30.0f,  150.0f, 15.0f, 10.0f, 0.5f, sf::Color::Green}},
            {EnemyType::Bruiser, {150.0f, 60.0f,  30.0f, 35.0f, 1.5f, sf::Color(128, 0, 128)}},
            {EnemyType::Spitter, {50.0f,  100.0f, 18.0f, 0.0f,  2.0f, sf::Color::Yellow}}
        };

        return stats.at(eType);
    }
};