#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"

#include <algorithm>

class Player : public Entity{
public:
    static std::unique_ptr<Player> create(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass)    ;

    Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass);
    virtual ~Player() = default;

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;

    void setFocused(bool focuesd);
    PlayerClass getClass() const { return m_class; }

    virtual void onShift(const sf::Vector2f& mouseWorldPos) {}
    virtual void onQ(const sf::Vector2f& mouseWorldPos) {}
    virtual void onE(const sf::Vector2f& mouseWorldPos) {}

    int getLevel() const { return m_level; }
    int getExp() const { return m_exp; }
    int getExpMax() const { return m_expMax; }

    void setExpData(int level, int exp, int expMax);

protected:
    bool checkCollision(const sf::Vector2f& pos, const std::shared_ptr<MapGenerator>& map);

    sf::CircleShape m_shape;
    float m_speed;
    bool m_isFocused;
    PlayerClass m_class;

    sf::Vector2f m_lastMoveDirection{1.0f, 0.0f};

    sf::Clock m_cooldownShift;
    sf::Clock m_cooldownQ;
    sf::Clock m_cooldownE;

    int m_level = 1;
    int m_exp = 0;
    int m_expMax = 10;
};


class ScoutPlayer : public Player {
public:
    using Player::Player;
    
    void onShift(const sf::Vector2f& mouseWorldPos) override;
};