#pragma once
#include "../entities/Player.hpp"
#include "../entities/Enemy.hpp"
#include "../entities/HealField.hpp"
#include "../entities/BlackHole.hpp"
#include "../entities/Decoy.hpp"
#include "../entities/MedicOrb.hpp"
#include "../entities/MedicBarrier.hpp"
#include "../entities/MedicDrone.hpp"
#include "../core/MapRenderer.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include "MapGenerator.hpp"
#include "HeroRegistry.hpp"
#include "UpgradeRegistry.hpp"
#include "Config.hpp"
#include "State.hpp"
#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <optional>
#include <memory>
#include <map>
#include <array>
#include <string>

/**
 * @brief Active gameplay state for an in-session multiplayer match.
 *
 * Simulates the local player, mirrors remote entities from server snapshots,
 * manages projectiles and ability world objects, handles combat input, and
 * renders the world plus HUD overlays including upgrade selection and session end.
 */
class GameState : public State {
public:
    GameState(ClientEngine& engine, std::uint32_t myPlayerId, PlayerClass myClass);
    ~GameState() override;


    // State Interface
    void handlePacket(PacketType type, sf::Packet& packet) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;
    void renderUI() override;

private:
    // Types
    enum class SessionEndReason {
        None,
        Death,
        Disconnected
    };
    struct ClientHealField {
        sf::Vector2f position;
        float radius;
        float duration;
    };


    void handleWorldState(sf::Packet& packet);


    // Local Player & Camera
    sf::View m_camera;
    std::unique_ptr<Player> m_player;
    sf::Vector2f m_lastSentPosition;
    PlayerClass m_selectedClass = PlayerClass::Soldier;


    // Remote Entities
    std::map<std::uint32_t, std::unique_ptr<Player>> m_otherPlayers;
    std::map<std::uint32_t, std::unique_ptr<Enemy>> m_enemies;
    std::map<std::uint32_t, sf::Vector2f> m_energyCells;


    // Ability World Objects
    std::map<std::uint32_t, std::unique_ptr<HealField>> m_healFields;
    std::map<std::uint32_t, std::unique_ptr<BlackHole>> m_blackHoles;
    std::map<std::uint32_t, std::unique_ptr<Decoy>> m_decoys;
    std::map<std::uint32_t, std::unique_ptr<MedicOrb>> m_medicOrbs;
    std::map<std::uint32_t, std::unique_ptr<MedicBarrier>> m_medicBarriers;
    std::map<std::uint32_t, std::unique_ptr<MedicDrone>> m_medicDrones;


    // Network Timers
    sf::Clock m_lastServerMessageTimer;
    sf::Clock m_heartbeatTimer;


    // Map & Projectiles
    std::shared_ptr<MapGenerator> m_map;
    std::unique_ptr<MapRenderer> m_mapRenderer;
    std::unique_ptr<ProjectileManager> m_projectileManager;


    // Team Progression & Upgrades
    int m_teamLevel = 1;
    int m_teamExp = 0;
    int m_teamExpMax = 10;
    bool m_isChoosingUpgrade = false;
    bool m_upgradeRevealPhase = false;
    std::array<std::string, 3> m_upgradeOffers{};
    std::string m_chosenUpgradeId;
    sf::Clock m_clientUpgradeTimer;

    // Session State
    SessionEndReason m_sessionEndReason = SessionEndReason::None;
};
