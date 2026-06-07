#include "Juggernaut.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include "AbilityRegistry.hpp"


/**
 * @brief Initializes Juggernaut stats and ability tuning from the registry.
 * @param id Unique network entity identifier.
 * @param startPos Initial world position.
 */
Juggernaut::Juggernaut(std::uint32_t id, const sf::Vector2f& startPos) : Player(id, startPos, PlayerClass::Juggernaut){
    const auto& a = AbilityRegistry::juggernaut();
    m_maxAmmo = 6;
    m_ammo = m_maxAmmo;
    m_reloadTime = 2.5f;
    m_fireRate = 0.8f;
    m_shiftCooldown = a.Dash.cooldown;
    m_maxCooldownShift = m_shiftCooldown;
    m_maxChargeDistance = AbilityRegistry::param(a.Dash, "chargeDistance", 350.f);
    m_chargeSpeedMultiplier = AbilityRegistry::param(a.Dash, "chargeSpeedMultiplier", 3.5f);
    m_repulsorRange = a.Repulsor.range;
    m_repulsorAngleSpan = AbilityRegistry::param(a.Repulsor, "angleSpanDegrees", 70.f) * (M_PI / 180.f);
    m_repulsorVfxDuration = AbilityRegistry::param(a.Repulsor, "vfxDuration", 0.2f);
    m_blackHoleMaxRange = AbilityRegistry::param(a.BlackHole, "spawnMaxRange", 200.f);
    m_ultDuration = a.Ult.duration;
    m_recoilForce = AbilityRegistry::param(a.Ult, "recoilForce", 300.f);
    m_dashCollisionBonus = AbilityRegistry::param(a.Dash, "collisionBonus", 15.f);
}


/**
 * @brief Updates charge movement, repulsor VFX, recoil, ult timer, and base state.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map used for wall collision checks.
 */
void Juggernaut::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){

    // --- Charge movement ---
    if(m_isCharging){
        float chargeSpeed = m_speed * m_chargeSpeedMultiplier;
        float distanceToMove = chargeSpeed * deltaTime.asSeconds();
        
        if(distanceToMove > m_chargeDistanceRemaining) distanceToMove = m_chargeDistanceRemaining;
        sf::Vector2f velocity = m_chargeDirection * distanceToMove;

        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        bool hitX = checkCollision(nextPosX, map);
        if(!hitX) m_position.x = nextPosX.x;

        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        bool hitY = checkCollision(nextPosY, map);
        if(!hitY) m_position.y = nextPosY.y;

        m_shape.setPosition(m_position);
        m_chargeDistanceRemaining -= distanceToMove;

        if(m_chargeDistanceRemaining <= 0.0f || (hitX && hitY)){
            m_isCharging = false;
            m_isFocused = true;
        } 
    }

    // --- Repulsor VFX ---
    if(m_repulsorVfxTimer > 0.0f){
        m_repulsorVfxTimer -= deltaTime.asSeconds();

        int points = 10;
        float angleSpan = m_repulsorAngleSpan;
        float baseAngle = std::atan2(m_repulsorAimDir.y, m_repulsorAimDir.x);
        std::uint8_t alpha = static_cast<std::uint8_t>(150.0f * (std::max(0.0f, m_repulsorVfxTimer) / m_repulsorVfxDuration));

        m_repulsorVfx.resize(points + 2);
        m_repulsorVfx[0].position = m_position;
        m_repulsorVfx[0].color = sf::Color(100, 200, 255, alpha);

        for(int i = 0; i <= points; ++i) {
            float currentAngle = baseAngle - (angleSpan/2.0f) + (angleSpan * static_cast<float>(i) / points);
            m_repulsorVfx[i+1].position = m_position + sf::Vector2f(std::cos(currentAngle) * m_repulsorRange, std::sin(currentAngle) * m_repulsorRange);
            m_repulsorVfx[i+1].color = sf::Color(100, 200, 255, 0);
        }
    }

    // --- Ult recoil ---
    if (m_recoilVelocity.lengthSquared() > 10.0f) {
        sf::Vector2f velocity = m_recoilVelocity * deltaTime.asSeconds();

        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        if (!checkCollision(nextPosX, map)) m_position.x = nextPosX.x;

        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        if (!checkCollision(nextPosY, map)) m_position.y = nextPosY.y;

        m_shape.setPosition(m_position);
        m_recoilVelocity -= m_recoilVelocity * 12.0f * deltaTime.asSeconds();
    }

    // --- Ult timer ---
    if (m_isUltActive) {
        m_ultTimer -= deltaTime.asSeconds();
        if (m_ultTimer <= 0.0f) {
            m_isUltActive = false;
        }
    }
    Player::update(deltaTime, map);
}

// ==========================================
// Ability inputs
// ==========================================

/**
 * @brief Starts a directional charge dash toward the cursor.
 * @param mouseWorldPos Target direction in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Juggernaut::onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownShift <= 0.0f && !m_isCharging){
        m_cooldownShift = m_maxCooldownShift;
        m_chargeDistanceRemaining = m_maxChargeDistance;
        m_isCharging = true;
        m_isFocused = false;
        m_dashedEnemies.clear();

        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();

        if (lenSq > 0.0f) {
            m_chargeDirection = dir / std::sqrt(lenSq);
        } else {
            m_chargeDirection = sf::Vector2f(1.0f, 0.0f);
        }

        if (engine.getServerAddress()) {
            sf::Packet packet;
            packet << PacketType::AbilityUsed << m_id << AbilityType::JuggernautDash << m_chargeDirection;
            (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

/**
 * @brief Activates the rapid-fire ultimate when fully charged.
 * @param mouseWorldPos Cursor position in world space.
 * @param engine Client engine.
 * @param projMgr Projectile manager.
 */
void Juggernaut::onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if (!m_isUltActive && m_ultCharge >= m_maxUltCharge) {
        m_isUltActive = true;
        m_ultTimer = m_ultDuration;
        m_ultCharge = 0.0f; 
    }
}

/**
 * @brief Requests a black hole spawn at the cursor, clamped to max range.
 * @param mouseWorldPos Desired spawn position in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Juggernaut::onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownE <= 0.0f){
        m_cooldownE = m_maxCooldownE;
        float maxRange = m_blackHoleMaxRange;

        sf::Vector2f targetPos = mouseWorldPos;
        sf::Vector2f dir = mouseWorldPos - m_position;
        float distSq = dir.lengthSquared();

        if(distSq > maxRange * maxRange){
            targetPos = m_position + (dir / std::sqrt(distSq)) * maxRange;
        }

        if(engine.getServerAddress()){
            sf::Packet packet;
            packet << PacketType::AbilityUsed << m_id << AbilityType::JuggernautBlackHole << targetPos;
            (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

/**
 * @brief Fires the frontal repulsor cone toward the cursor.
 * @param mouseWorldPos Aim direction in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager.
 */
void Juggernaut::onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownRMB <= 0.0f){
        m_cooldownRMB = m_maxCooldownRMB;
        m_repulsorVfxTimer = m_repulsorVfxDuration;
        m_fireRepulsor = true;

        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();

        if (lenSq > 0.0f) m_repulsorAimDir = dir / std::sqrt(lenSq);

        if (engine.getServerAddress()) {
            sf::Packet packet;
            packet << PacketType::AbilityUsed << m_id << AbilityType::JuggernautRepulsor << m_repulsorAimDir;
            (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

/**
 * @brief Fires the shotgun and applies recoil knockback during the ultimate.
 * @param mouseWorldPos Target position in world space.
 * @param engine Client engine used to notify the server.
 * @param projMgr Projectile manager that spawns shotgun pellets.
 * @param enemies Enemy map.
 */
void Juggernaut::onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies){
    if (m_cooldownLMB > 0.0f || m_isReloading) return; 
    if (m_ammo == 0 && !m_isUltActive) {
        reload();
        return;
    }

    if (m_isUltActive) {
        m_cooldownLMB = (m_fireRate * m_fireRateMultiplier) * 0.25f;
    } else {
        m_cooldownLMB = m_fireRate * m_fireRateMultiplier;
        m_ammo--;
    }

    WeaponType weapon = WeaponType::Shotgun;
    projMgr.spawnProjectile(m_id, m_position, mouseWorldPos, weapon, Faction::Players);
    if (engine.getServerAddress()) {
        sf::Packet shootPacket;
        shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << mouseWorldPos;
        (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
    }

    if (m_isUltActive) {
        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();
        if (lenSq > 0) {
            sf::Vector2f dirNormalized = dir / std::sqrt(lenSq);
            m_recoilVelocity += -dirNormalized * m_recoilForce; 
        }
    }
}

// ==========================================
// Remote sync
// ==========================================

/**
 * @brief Replays remote charge dash or repulsor visuals from network data.
 * @param ability Ability type that was used remotely.
 * @param data Direction or aim vector supplied by the server.
 */
void Juggernaut::playRemoteAbility(AbilityType ability, const sf::Vector2f& data) {
    if (ability == AbilityType::JuggernautDash && !m_isCharging) {
        m_isCharging = true;
        m_chargeDistanceRemaining = m_maxChargeDistance;
        m_isFocused = false;
        m_dashedEnemies.clear();
        m_chargeDirection = data;

        float len = m_chargeDirection.length();

        if (len > 0.0001f) m_chargeDirection /= len;
        else m_chargeDirection = sf::Vector2f(1.0f, 0.0f);

    } else if (ability == AbilityType::JuggernautRepulsor) {
        m_repulsorAimDir = data;
        m_repulsorVfxTimer = m_repulsorVfxDuration;

        float len = m_repulsorAimDir.length();
        
        if (len > 0.0001f) m_repulsorAimDir /= len;
        else m_repulsorAimDir = sf::Vector2f(1.0f, 0.0f);
    }
}

/**
 * @brief Updates repulsor VFX for a remotely controlled Juggernaut.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map reference.
 */
void Juggernaut::updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) {
    float dt = deltaTime.asSeconds();

    if (m_repulsorVfxTimer > 0.0f) {
        m_repulsorVfxTimer -= dt;

        int points = 10;
        float angleSpan = m_repulsorAngleSpan;
        float baseAngle = std::atan2(m_repulsorAimDir.y, m_repulsorAimDir.x);
        std::uint8_t alpha = static_cast<std::uint8_t>(150.0f * (std::max(0.0f, m_repulsorVfxTimer) / m_repulsorVfxDuration));

        m_repulsorVfx.resize(points + 2);
        m_repulsorVfx[0].position = m_position;
        m_repulsorVfx[0].color = sf::Color(100, 200, 255, alpha);

        for (int i = 0; i <= points; ++i) {
            float currentAngle = baseAngle - (angleSpan / 2.0f) + (angleSpan * static_cast<float>(i) / points);
            m_repulsorVfx[i + 1].position = m_position + sf::Vector2f(std::cos(currentAngle) * m_repulsorRange, std::sin(currentAngle) * m_repulsorRange);
            m_repulsorVfx[i + 1].color = sf::Color(100, 200, 255, 0);
        }
    }
}

// ==========================================
// Ability hit detection
// ==========================================

/**
 * @brief Detects charge dash and repulsor hits against nearby enemies.
 * @param entities All entities to test for overlap or cone inclusion.
 * @return Records of enemies hit by active abilities this frame.
 */
std::vector<AbilityHitRecord> Juggernaut::checkAbilityHits(const std::vector<Entity*>& entities){
    std::vector<AbilityHitRecord> hits;
    if(m_isCharging){

        // --- Charge hits ---
        for(Entity* entity : entities){
            if(entity->getFaction() == Faction::Players) continue;

            std::uint32_t id = entity->getId();
            if(std::find(m_dashedEnemies.begin(), m_dashedEnemies.end(), id) != m_dashedEnemies.end()) continue;

            sf::Vector2f diff = m_position - entity->getPosition();
            float distSq = diff.lengthSquared();
            float collDist = getRadius() + entity->getRadius() + m_dashCollisionBonus;

            if(distSq < collDist * collDist) {
                m_dashedEnemies.push_back(id);;
                hits.push_back({id, AbilityType::JuggernautDash});
            }
        }
    }else if(m_fireRepulsor){

        // --- Repulsor hits ---
        float range = m_repulsorRange;
        float angleSpan = m_repulsorAngleSpan;
        float cosHalfAngle = std::cos(angleSpan / 2.0f);

        for(Entity* entity : entities){
            if(entity->getFaction() == Faction::Players) continue;

            sf::Vector2f diff = entity->getPosition() - m_position;
            float distSq = diff.lengthSquared();

            if (distSq > 0.0f && distSq < range * range) {
                sf::Vector2f dirToEnemy = diff / std::sqrt(distSq);
                float dot = dirToEnemy.x * m_repulsorAimDir.x + dirToEnemy.y * m_repulsorAimDir.y;
                
                if(dot >= cosHalfAngle){
                    hits.push_back({entity->getId(), AbilityType::JuggernautRepulsor});
                }
            }
        }
        m_fireRepulsor = false;
    }
    return hits;
}

// ==========================================
// Render
// ==========================================

/**
 * @brief Draws the repulsor cone VFX and the base player representation.
 * @param target Render target to draw into.
 */
void Juggernaut::render(sf::RenderTarget& target){
    if (m_repulsorVfxTimer > 0.0f) {
        target.draw(m_repulsorVfx);
    }
    Player::render(target);
}
