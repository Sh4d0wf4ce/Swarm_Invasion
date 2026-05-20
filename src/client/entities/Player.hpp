#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "WeaponRegistry.hpp"

#include <algorithm>

class Player : public Entity{
public:
    static std::unique_ptr<Player> create(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass)    ;

    Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass);
    virtual ~Player() = default;

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;
    virtual void renderUI() {};

    void setFocused(bool focuesd);
    PlayerClass getClass() const { return m_class; }
    Faction getFaction()  const override { return Faction::Players; }
    float getRadius() const override { return HeroRegistry::getStats(m_class).radius; }


    virtual void onShift(const sf::Vector2f& mouseWorldPos) {}
    virtual void onQ(const sf::Vector2f& mouseWorldPos) {}
    virtual void onE(const sf::Vector2f& mouseWorldPos) {}

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

    float m_speedMultiplier{1.0f};
};


class ScoutPlayer : public Player {
public:
    using Player::Player;
    
    void onShift(const sf::Vector2f& mouseWorldPos) override;
};