#include "Juggernaut.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"

Juggernaut::Juggernaut(std::uint32_t id, const sf::Vector2f& startPos) : Player(id, startPos, PlayerClass::Juggernaut){
    m_maxAmmo = 6;
    m_ammo = m_maxAmmo;
    m_reloadTime = 2.5f;
    m_fireRate = 0.8f;
    m_maxCooldownShift = 5.0f;
}

void Juggernaut::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    if(m_isCharging){
        float chargeSpeed = m_speed * 3.5f;
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

    Player::update(deltaTime, map);

    if(m_repulsorVfxTimer > 0.0f){
        m_repulsorVfxTimer -= deltaTime.asSeconds();

        int points = 10;
        float angleSpan = 70.0f * (M_PI / 180.0f);
        float baseAngle = std::atan2(m_repulsorAimDir.y, m_repulsorAimDir.x);

        m_repulsorVfx.resize(points + 2);

        std::uint8_t alpha = static_cast<std::uint8_t>(150.0f * (std::max(0.0f, m_repulsorVfxTimer) / 0.2f));

        m_repulsorVfx[0].position = m_position;
        m_repulsorVfx[0].color = sf::Color(100, 200, 255, alpha);

        for(int i = 0; i <= points; ++i) {
            float currentAngle = baseAngle - (angleSpan/2.0f) + (angleSpan * static_cast<float>(i) / points);
            m_repulsorVfx[i+1].position = m_position + sf::Vector2f(std::cos(currentAngle) * 250.0f, std::sin(currentAngle) * 250.0f);
            m_repulsorVfx[i+1].color = sf::Color(100, 200, 255, 0);
        }
    }
}

void Juggernaut::onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownShift <= 0.0f && !m_isCharging){
        m_cooldownShift = m_maxCooldownShift;

        m_isCharging = true;
        m_chargeDistanceRemaining = m_maxChargeDistance;
        m_isFocused = false;
        m_dashedEnemies.clear();
        
        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();
        if (lenSq > 0.0f) {
            m_chargeDirection = dir / std::sqrt(lenSq);
        } else {
            m_chargeDirection = sf::Vector2f(1.0f, 0.0f);
        }
    }
}

void Juggernaut::onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){

}

void Juggernaut::onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){

}

void Juggernaut::onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownRMB <= 0.0f){
        m_cooldownRMB = m_maxCooldownRMB;

        m_fireRepulsor = true;

        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();
        if (lenSq > 0.0f) m_repulsorAimDir = dir / std::sqrt(lenSq);

        m_repulsorVfxTimer = 0.2f;
    }
}

void Juggernaut::onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies){
    if(m_ammo > 0 && !m_isReloading && m_cooldownLMB <= 0.0f){
        m_cooldownLMB = m_fireRate * m_fireRateMultiplier;

        if(!m_isUltActive) m_ammo--;

        WeaponType weapon = WeaponType::Shotgun;

        projMgr.spawnProjectile(m_id, m_position, mouseWorldPos, weapon, Faction::Players);
        
        if (engine.getServerAddress()) {
            sf::Packet shootPacket;
            shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << mouseWorldPos;
            (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    } else if (m_ammo == 0 && !m_isReloading){
        reload();
    }
}

std::vector<AbilityHitRecord> Juggernaut::checkAbilityHits(const std::vector<Entity*>& entities){
    std::vector<AbilityHitRecord> hits;

    if(m_isCharging){
        for(Entity* entity : entities){
            if(entity->getFaction() == Faction::Players) continue;

            std::uint32_t id = entity->getId();

            if(std::find(m_dashedEnemies.begin(), m_dashedEnemies.end(), id) != m_dashedEnemies.end()) continue;

            sf::Vector2f diff = m_position - entity->getPosition();
            float distSq = diff.lengthSquared();
            float collDist = getRadius() + entity->getRadius() + 15.0f;

            if(distSq < collDist * collDist) {
                m_dashedEnemies.push_back(id);;
                hits.push_back({id, AbilityType::JuggernautDash});
            }
        }
    }else if(m_fireRepulsor){

        float range = 250.0f;
        float angleSpan = 70.0f * (M_PI / 180.0f);
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

void Juggernaut::render(sf::RenderTarget& target){
    if (m_repulsorVfxTimer > 0.0f) {
        target.draw(m_repulsorVfx);
    }
    Player::render(target);
}