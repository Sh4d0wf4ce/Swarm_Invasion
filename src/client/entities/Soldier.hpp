#pragma once

#include "Player.hpp"

class Soldier : public Player{
public:
    using Player::Player;

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void renderUI() override;

    void onShift(const sf::Vector2f& mouseWorldPos) override;
    void onQ(const sf::Vector2f& mouseWorldPos) override;
    void onE(const sf::Vector2f& mouseWorldPos) override;

    float getStamina() const { return m_stamina; }
    float getMaxStamina() const {return m_maxStamina; }
    bool isExhausted() const { return m_isExhausted; }

private:
    float m_stamina{100.0f};
    float m_maxStamina{100.0f};
    bool m_isExhausted{false};
    bool m_isSprinting{false};
};