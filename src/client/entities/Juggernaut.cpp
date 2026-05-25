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
    if( m_isCharging){
        m_chargeTimer -= deltaTime.asSeconds();

        if(m_chargeTimer <= 0.0f){
            m_isCharging = false;
            m_isFocused = true;
        }else{
            float chargeSpeed = m_speed * 3.0f;
            sf::Vector2f velocity = m_chargeDirection * chargeSpeed * deltaTime.asSeconds();

            sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
            if (!checkCollision(nextPosX, map)) m_position.x = nextPosX.x;

            sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
            if (!checkCollision(nextPosY, map)) m_position.y = nextPosY.y;

            m_shape.setPosition(m_position);
        }
    }

    Player::update(deltaTime, map);
}

void Juggernaut::onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownShift <= 0.0f && !m_isCharging){
        m_cooldownShift = m_maxCooldownShift;

        m_isCharging = true;
        m_chargeTimer = m_chargeDuartion;
        m_isFocused = false;
        
        float lenSq = m_lastMoveDirection.lengthSquared();
        if (lenSq > 0.0f) {
            m_chargeDirection = m_lastMoveDirection / std::sqrt(lenSq);
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
