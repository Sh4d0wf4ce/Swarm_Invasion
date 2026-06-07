#pragma once
#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include "WeaponRegistry.hpp"
#include "ServerTypes.hpp"
#include "AIDirector.hpp"
#include "Config.hpp"
#include "UpgradeRegistry.hpp"
#include <SFML/System.hpp>
#include <memory>
#include <random>
#include <map>
#include <vector>
#include <string>

/**
 * @brief Phase of the team level-up upgrade selection menu.
 */
enum class UpgradeMenuPhase {
    None,
    Selecting,
    Reveal
};

/**
 * @brief Authoritative UDP game server managing clients, combat, abilities, and world state.
 *
 * Runs a fixed 60 Hz tick loop, processes incoming packets, simulates enemies and ability
 * entities, handles upgrade progression, and broadcasts world snapshots to connected clients.
 */
class ServerEngine{
public:
    ServerEngine();
    void run();
    
private:
    // ==========================================
    // Tick & Network Loop
    // ==========================================
    void processNetwork();
    void update(sf::Time deltaTime);
    void sendWorldState();

    // ==========================================
    // Packet Handlers
    // ==========================================
    void handlePing(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port);
    void handlePlayerPosition(sf::Packet& packet);
    void handleEntityHit(sf::Packet& packet);
    void handleJoinRequest(sf::Packet& packet, const sf::IpAddress& sender, unsigned short port);
    void handlePlayerShoots(sf::Packet& packet);
    void handlePlayerDisconnect(sf::Packet& packet);
    void handleUpgradeChosen(sf::Packet& packet);
    void handleAbilityHit(sf::Packet& packet);
    void handleAbilityUsed(sf::Packet& packet);

    // ==========================================
    // Session & World Management
    // ==========================================
    void proccessUpgradeMenuTimeout();
    void removeAFKPlayers();
    bool needsWorldReset() const;
    void resetWorld();

    // ==========================================
    // Upgrade System
    // ==========================================
    float getEffectiveMaxHp(const ClientInfo& client) const;
    std::vector<const UpgradeDefinition*> buildUpgradePool(PlayerClass pClass, bool wantAugment) const;
    std::vector<std::string> rollUpgradeOffer(const ClientInfo& client) const;
    void applyUpgrade(ClientInfo& client, const UpgradeDefinition& upgrade);
    void sendLevelUpOffers();
    void sendUpgradeMultipliers(std::uint32_t playerId);
    void finalizeUpgradeSelections();

    // ==========================================
    // Ability & Entity Simulation
    // ==========================================
    void updateEnergyCells(sf::Time deltaTime);
    void updateHealingFields(sf::Time deltaTime);
    void updateMedicPassives(sf::Time deltaTime);
    void updateBlackHoles(sf::Time deltaTime);
    void updateMedicOrbs(sf::Time deltaTime);
    void updateMedicBarriers(sf::Time deltaTime);
    void updateMedicDrones(sf::Time deltaTime);
    void updateServerProjectiles(sf::Time deltaTime);

    // ==========================================
    // Medic Drone & Projectile Helpers
    // ==========================================
    void handleMedicDroneCommand(std::uint32_t playerId, sf::Vector2f targetPos);
    void spawnDroneBlaster(std::uint32_t ownerId, sf::Vector2f startPos, sf::Vector2f targetPos);
    void spawnMedicNeedle(std::uint32_t ownerId, sf::Vector2f startPos, sf::Vector2f targetPos);
    std::uint32_t findNearestEnemyId(const sf::Vector2f& from, float maxRange) const;
    MedicDroneData* findDroneByOwner(std::uint32_t ownerId);

    // ==========================================
    // Decoy Helpers
    // ==========================================
    void explodeDecoy(std::uint32_t decoyId, const DecoyData& decoy);
    bool playerHasActiveDecoy(std::uint32_t playerId) const;
    bool playerHasActiveOrb(std::uint32_t playerId) const;

    // ==========================================
    // Core Infrastructure
    // ==========================================
    std::shared_ptr<MapGenerator> m_map;
    AIDirector m_aiDirector;
    sf::UdpSocket m_socket;
    sf::Clock m_clock;
    sf::Time m_timePerTick;
    bool m_isRunning;
    int  m_tickCounter;
    std::uint32_t m_globalEntityCounter = 1;

    // ==========================================
    // Connected Clients & World Entities
    // ==========================================
    std::map<std::uint32_t, ClientInfo> m_clients;
    std::map<std::uint32_t, EnergyCellInfo> m_energyCells;
    std::map<std::uint32_t, HealFieldInfo> m_healFields;
    std::map<std::uint32_t, BlackHoleData> m_blackHoles;
    std::map<std::uint32_t, DecoyData> m_decoys;
    std::map<std::uint32_t, MedicOrbData> m_medicOrbs;
    std::map<std::uint32_t, MedicBarrierData> m_medicBarriers;
    std::map<std::uint32_t, MedicDroneData> m_medicDrones;
    std::vector<ServerProjectileData> m_serverProjectiles;
    std::map<std::uint32_t, std::unique_ptr<ServerEnemy>> m_enemies;

    // ==========================================
    // Team Progression & Upgrade Menu
    // ==========================================
    int m_teamLevel = 1;
    int m_teamExp = 0;
    int m_teamExpMax = 10;
    bool m_isPaused = false;
    sf::Clock m_upgradeTimer;
    sf::Clock m_upgradeRevealTimer;
    UpgradeMenuPhase m_upgradeMenuPhase = UpgradeMenuPhase::None;
    std::map<std::uint32_t, std::string> m_playerChosenUpgrades;
    std::map<std::uint32_t, std::vector<std::string>> m_pendingOffers;
};
