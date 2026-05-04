#pragma once

#include "Player.hpp"
#include "Enemy.hpp"
#include "MapGenerator.hpp"
#include "MapRenderer.hpp"
#include "ProjectileManager.hpp"
#include "HeroRegistry.hpp"
#include "ClientEngine.hpp"
#include "Config.hpp"
#include "State.hpp"

#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <optional>
#include <memory>
#include <map>

class GameState : public State {
public:
    GameState(ClientEngine& engine, std::uint32_t myPlayerId, PlayerClass myClass);
    ~GameState() override = default;

    void handlePacket(PacketType type, sf::Packet& packet) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;
    void renderUI() override;

private:
    void handleWorldState(sf::Packet& packet);

    sf::View m_camera;

    std::unique_ptr<Player> m_player;
    sf::Vector2f m_lastSentPosition;

    std::map<std::uint32_t, std::unique_ptr<Player>> m_otherPlayers;
    std::map<std::uint32_t, std::unique_ptr<Enemy>> m_enemies;
    
    sf::Clock m_lastServerMessageTimer;
    sf::Clock m_heartbeatTimer;

    std::shared_ptr<MapGenerator> m_map;
    std::unique_ptr<MapRenderer> m_mapRenderer;
    std::unique_ptr<ProjectileManager> m_projectileManager;

    PlayerClass m_selectedClass = PlayerClass::Soldier;
};