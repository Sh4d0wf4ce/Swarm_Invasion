#include "Medic.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include "AbilityRegistry.hpp"
#include "imgui.h"


/**
 * @brief Initializes Medic ability cooldowns and ultimate charge requirement.
 * @param id Unique network entity identifier.
 * @param startPos Initial world position.
 */
Medic::Medic(std::uint32_t id, const sf::Vector2f& startPos): Player(id, startPos, PlayerClass::Medic) {
    m_maxAmmo = 0;
    m_ammo = 0;
    m_fireRate = 0.2f;
    const auto& m = AbilityRegistry::medic();
    m_maxCooldownShift = m.Teleport.cooldown;
    m_maxCooldownRMB = m.Orb.cooldown;
    m_maxCooldownE = m.Barrier.cooldown;
    m_maxUltCharge = AbilityRegistry::param(m.Drone, "ultChargeRequired", 10.f);
}

// ==========================================
// Teleport system
// ==========================================

/**
 * @brief Computes a valid teleport destination clamped to ability range.
 * @param mouseWorldPos Desired cursor position in world space.
 * @param map Tile map used to reject wall collisions.
 * @param outTarget Receives the resolved teleport position on success.
 * @return True when the target position is collision-free.
 */
bool Medic::computeTeleportTarget(const sf::Vector2f& mouseWorldPos, const std::shared_ptr<MapGenerator>& map, sf::Vector2f& outTarget) {
    const float teleportRange = AbilityRegistry::medic().Teleport.range;
    sf::Vector2f dir = mouseWorldPos - m_position;
    float distSq = dir.lengthSquared();

    if (distSq > teleportRange * teleportRange) {
        float dist = std::sqrt(distSq);
        outTarget = m_position + (dir / dist) * teleportRange;
    } else {
        outTarget = mouseWorldPos;
    }

    if (checkCollision(outTarget, map)) return false;
    return true;
}

/**
 * @brief Begins the teleport fade-out animation when Shift input is valid.
 * @param map Tile map used to validate the destination.
 */
void Medic::tryStartTeleport(const std::shared_ptr<MapGenerator>& map) {
    if (m_cooldownShift > 0.0f || m_teleportPhase != TeleportPhase::None) return;

    sf::Vector2f target;
    if (!computeTeleportTarget(m_shiftMousePos, map, target)) return;

    m_teleportTarget = target;
    m_cooldownShift = m_maxCooldownShift;
    m_teleportAnimTime = 0.0f;
    m_teleportPhase = TeleportPhase::FadeOut;
    const auto& stats = HeroRegistry::getStats(m_class);
    m_baseFillColor = stats.color;
    m_baseOutlineColor = stats.color;

    if (m_shiftEngine && m_shiftEngine->getServerAddress()) {
        sf::Packet packet;
        packet << PacketType::AbilityUsed << m_id << AbilityType::MedicTeleport << m_teleportTarget;
        (void)m_shiftEngine->getSocket().send(packet, m_shiftEngine->getServerAddress().value(), Config::SERVER_PORT);
    }
}

/**
 * @brief Advances the fade-out and fade-in teleport animation phases.
 * @param dt Elapsed time in seconds since the last update.
 */
void Medic::advanceTeleportAnimation(float dt) {
    m_teleportAnimTime += dt;
    const float half = AbilityRegistry::param(AbilityRegistry::medic().Teleport, "fadeTotal", 0.4f) / 2.0f;

    if (m_teleportPhase == TeleportPhase::FadeOut) {
        const float t = std::min(m_teleportAnimTime, half);
        const std::uint8_t alpha = static_cast<std::uint8_t>(255.0f * (1.0f - t / half));
        m_shape.setFillColor(sf::Color(m_baseFillColor.r, m_baseFillColor.g, m_baseFillColor.b, alpha));
        m_shape.setOutlineColor(sf::Color(m_baseOutlineColor.r, m_baseOutlineColor.g, m_baseOutlineColor.b, alpha));

        if (m_teleportAnimTime >= half) {
            setPosition(m_teleportTarget);
            m_shape.setPosition(m_position);
            m_teleportPhase = TeleportPhase::FadeIn;
            m_teleportAnimTime = 0.0f;
        }
    } else {
        const float t = std::min(m_teleportAnimTime, half);
        const std::uint8_t alpha = static_cast<std::uint8_t>(255.0f * (t / half));
        m_shape.setFillColor(sf::Color(m_baseFillColor.r, m_baseFillColor.g, m_baseFillColor.b, alpha));
        m_shape.setOutlineColor(sf::Color(m_baseOutlineColor.r, m_baseOutlineColor.g, m_baseOutlineColor.b, alpha));

        if (m_teleportAnimTime >= half) {
            m_shape.setFillColor(m_baseFillColor);
            m_shape.setOutlineColor(m_baseOutlineColor);
            m_teleportPhase = TeleportPhase::None;
        }
    }
    m_shape.setPosition(m_position);
}


/**
 * @brief Processes deferred teleport input and animation, otherwise updates movement.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map used for teleport validation and collision.
 */
void Medic::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) {

    // --- Shift teleport ---
    if (m_shiftRequested) {
        m_shiftRequested = false;
        tryStartTeleport(map);
        m_shiftEngine = nullptr;
    }

    // --- Teleport animation ---
    if (m_teleportPhase != TeleportPhase::None) {
        float dt = deltaTime.asSeconds();
        if (m_cooldownShift > 0.0f) m_cooldownShift -= dt;
        if (m_cooldownE > 0.0f) m_cooldownE -= dt;
        if (m_cooldownRMB > 0.0f) m_cooldownRMB -= dt;
        if (m_cooldownLMB > 0.0f) m_cooldownLMB -= dt;
        advanceTeleportAnimation(dt);
        return;
    }
    Player::update(deltaTime, map);
}


/**
 * @brief Draws the Medic using the base player renderer.
 * @param target Render target to draw into.
 */
void Medic::render(sf::RenderTarget& target) {
    Player::render(target);
}


// ==========================================
// Ability inputs
// ==========================================

/**
 * @brief Fires a healing needle projectile toward the cursor.
 * @param mouseWorldPos Target position in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager that spawns the needle.
 * @param enemies Enemy map.
 */
void Medic::onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) {
    if (m_cooldownLMB > 0.0f) return;

    m_cooldownLMB = m_fireRate * m_fireRateMultiplier;
    WeaponType weapon = WeaponType::MedicNeedle;
    projMgr.spawnProjectile(m_id, m_position, mouseWorldPos, weapon, Faction::Players);

    if (engine.getServerAddress()) {
        sf::Packet shootPacket;
        shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << mouseWorldPos;
        (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

/**
 * @brief Launches a healing orb in the cursor direction.
 * @param mouseWorldPos Aim direction in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Medic::onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (m_cooldownRMB > 0.0f) return;
    m_cooldownRMB = m_maxCooldownRMB;

    sf::Vector2f dir = mouseWorldPos - m_position;
    float lenSq = dir.lengthSquared();

    if (lenSq < 1e-4f) {
        dir = sf::Vector2f(1.0f, 0.0f);
    } else {
        dir /= std::sqrt(lenSq);
    }

    if (engine.getServerAddress()) {
        sf::Packet packet;
        packet << PacketType::AbilityUsed << m_id << AbilityType::MedicOrb << dir;
        (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

/**
 * @brief Queues a teleport request to be processed on the next update.
 * @param mouseWorldPos Desired destination in world space.
 * @param engine Client engine stored for deferred network dispatch.
 * @param projMgr Projectile manager.
 */
void Medic::onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (m_cooldownShift > 0.0f || m_teleportPhase != TeleportPhase::None) return;

    m_shiftRequested = true;
    m_shiftMousePos = mouseWorldPos;
    m_shiftEngine = &engine;
}

/**
 * @brief Places a directional barrier wall facing the cursor.
 * @param mouseWorldPos Aim direction in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Medic::onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (m_cooldownE > 0.0f) return;
    m_cooldownE = m_maxCooldownE;

    sf::Vector2f dir = mouseWorldPos - m_position;
    float facingAngle = std::atan2(dir.y, dir.x);

    if (engine.getServerAddress()) {
        sf::Packet packet;
        packet << PacketType::AbilityUsed << m_id << AbilityType::MedicBarrier << m_position << facingAngle;
        (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

/**
 * @brief Deploys or commands the healing drone ultimate.
 * @param mouseWorldPos Command target position in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Medic::onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    const float droneLifetime = AbilityRegistry::medic().Drone.lifetime;

    if (!m_droneActive) {
        if (m_ultCharge < m_maxUltCharge) return;
        m_ultCharge = 0.0f;
        m_isUltActive = true;
        m_droneActive = true;
        m_droneLifetime = droneLifetime;
    }

    if (engine.getServerAddress()) {
        sf::Packet packet;
        packet << PacketType::AbilityUsed << m_id << AbilityType::MedicUltCommand << mouseWorldPos;
        (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

// ==========================================
// Drone state
// ==========================================

/**
 * @brief Synchronizes local drone active state and remaining lifetime.
 * @param active True when the drone is currently deployed.
 * @param lifetime Remaining drone duration in seconds.
 */
void Medic::setDroneState(bool active, float lifetime) {
    m_droneActive = active;
    m_droneLifetime = lifetime;
    m_isUltActive = active;

    if (!active) {
        m_droneLifetime = 0.0f;
    }
}

/**
 * @brief Renders the drone lifetime or ultimate charge bar in the HUD.
 */
void Medic::renderQSkill() {
    ImGui::Text("Q");
    const float droneLifetime = AbilityRegistry::medic().Drone.lifetime;
    if (m_droneActive) {
        char timerText[16];
        sprintf(timerText, "%.1fs", std::max(0.0f, m_droneLifetime));
        float progress = m_droneLifetime / droneLifetime;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.9f, 0.7f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), timerText);
        ImGui::PopStyleColor();
        return;
    }

    float progress = m_ultCharge / m_maxUltCharge;
    if (progress >= 1.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.9f, 0.7f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        char percText[16];
        sprintf(percText, "%.0f%%", progress * 100.0f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), percText);
        ImGui::PopStyleColor();
    }
}


/**
 * @brief Medic abilities do not register local hit detection on the client.
 * @param entities All entities to test.
 * @return Empty hit list.
 */
std::vector<AbilityHitRecord> Medic::checkAbilityHits(const std::vector<Entity*>& entities) {
    return {};
}

// ==========================================
// Remote sync
// ==========================================

/**
 * @brief Starts the teleport fade animation for a remote Medic.
 * @param ability Ability type that was used remotely.
 * @param data Teleport destination supplied by the server.
 */
void Medic::playRemoteAbility(AbilityType ability, const sf::Vector2f& data) {
    if (ability != AbilityType::MedicTeleport) return;

    const auto& stats = HeroRegistry::getStats(m_class);
    m_teleportTarget = data;
    m_baseFillColor = stats.color;
    m_baseOutlineColor = stats.color;
    m_teleportPhase = TeleportPhase::FadeOut;
    m_teleportAnimTime = 0.0f;
}

/**
 * @brief Advances teleport animation for a remotely controlled Medic.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map reference.
 */
void Medic::updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) {
    if (m_teleportPhase != TeleportPhase::None) {
        advanceTeleportAnimation(deltaTime.asSeconds());
    }
}
