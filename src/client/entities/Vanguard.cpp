#include "Vanguard.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include <cmath>
#include <algorithm>

Vanguard::Vanguard(std::uint32_t id, const sf::Vector2f& startPos)
    : Player(id, startPos, PlayerClass::Vanguard) {
    m_maxAmmo = 0;
    m_ammo = 0;
    m_fireRate = 0.2f;
    m_speed = 220.0f;
    m_maxCooldownE = 8.0f;
}

void Vanguard::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) {
    Player::update(deltaTime, map);
    float dt = deltaTime.asSeconds();

    if (getStealthTimer() > 0.0f) {
        m_shape.setFillColor(sf::Color(0, 255, 255, 80));
        m_shape.setOutlineColor(sf::Color(0, 255, 255, 120));
    } else {
        m_shape.setFillColor(HeroRegistry::getStats(m_class).color);
        m_shape.setOutlineColor(HeroRegistry::getStats(m_class).color);
    }

    if (m_isUltActive) {
        m_ultTimer -= dt;
        if (m_ultTimer <= 0.0f) {
            m_isUltActive = false;
            m_speed /= 2.0f;
        }
    }

    if (m_shurikensToFire > 0 && m_engineRef && m_projMgrRef) {
        m_shurikenBurstTimer -= dt;
        
        if (m_shurikenBurstTimer <= 0.0f) {
            m_shurikenBurstTimer = 0.08f;

            float baseAngle = std::atan2(m_shurikenAimDir.y, m_shurikenAimDir.x);

            int index = 5 - m_shurikensToFire; 
            
            float angleOffset = (index - 2) * 5.0f * (M_PI / 180.0f);
            float finalAngle = baseAngle + angleOffset;

            sf::Vector2f targetOffset(std::cos(finalAngle), std::sin(finalAngle));
            sf::Vector2f finalTarget = m_position + targetOffset * 1000.0f; 

            WeaponType weapon = WeaponType::Shuriken;
            m_projMgrRef->spawnProjectile(m_id, m_position, finalTarget, weapon, Faction::Players);

            if (m_engineRef->getServerAddress()) {
                sf::Packet shootPacket;
                shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << finalTarget;
                (void)m_engineRef->getSocket().send(shootPacket, m_engineRef->getServerAddress().value(), Config::SERVER_PORT);
            }

            m_shurikensToFire--;
        }
    }

    if(m_dashCharges < m_maxDashCharges){
        m_dashRechargeTimer += dt;
        if(m_dashRechargeTimer >= m_dashRechargeTime){
            m_dashCharges++;
            m_dashRechargeTimer = 0.0f;
        }
    }

    if(m_isDashing){
        float dashSpeed = 2500.0f;
        float moveDist = dashSpeed * dt;
        if(moveDist > m_dashDistanceRemaining) moveDist = m_dashDistanceRemaining;

        sf::Vector2f velocity = m_dashDir * moveDist;
        
        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        bool hitX = checkCollision(nextPosX, map);
        if (!hitX) m_position.x = nextPosX.x;

        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        bool hitY = checkCollision(nextPosY, map);
        if (!hitY) m_position.y = nextPosY.y;

        m_shape.setPosition(m_position);
        m_dashDistanceRemaining -= moveDist;

        if(m_dashDistanceRemaining <= 0.0f || (hitX && hitY)){
            m_isDashing = false;
            m_isFocused = true;

            m_dashTrails.push_back({m_dashStartPos, m_position, 0.0f});
        }
    }

    for (auto& trail : m_dashTrails) trail.age += deltaTime.asSeconds();
    m_dashTrails.erase(std::remove_if(m_dashTrails.begin(), m_dashTrails.end(),
        [](const DashTrail& t) { return t.age > 0.35f; }), m_dashTrails.end());

    updateTrail(dt);

    if (!m_attackActive) return;

    m_attackTime += dt;
    if (m_attackTime >= m_swingDuration) {
        m_attackActive = false;
        m_attackTime = 0.0f;
        m_cooldownLMB = m_fireRate * m_fireRateMultiplier;
        return;
    }

    float progress = std::clamp(m_attackTime / m_swingDuration, 0.0f, 1.0f);
    float t = progress * progress * (3.0f - 2.0f * progress);

    m_prevBladeAngle = m_bladeAngle;
    m_bladeAngle = getBladeAngleAt(t);
    m_trail.push_back({m_bladeAngle, 0.0f});
}


std::vector<AbilityHitRecord> Vanguard::checkAbilityHits(const std::vector<Entity*>& entities) {
    std::vector<AbilityHitRecord> hits;
    if (m_attackActive){
        for (Entity* e : entities) {
            if (e->getFaction() == Faction::Players) continue;
            if (m_hitThisSwing.count(e->getId())) continue;

            sf::Vector2f diff = e->getPosition() - m_position;
            float distSq = diff.lengthSquared();
            
            float maxReach = m_outerRadius + e->getRadius();
            float minReach = std::max(0.0f, m_innerRadius - e->getRadius());
            
            if (distSq > maxReach * maxReach || distSq < minReach * minReach) continue;


            float enemyAngle = std::atan2(diff.y, diff.x);
            float angleMargin = std::asin(std::clamp(e->getRadius() / std::sqrt(distSq), 0.0f, 1.0f));

            if (isAngleBetween(enemyAngle, m_prevBladeAngle - angleMargin, m_bladeAngle + angleMargin) || 
                isAngleBetween(enemyAngle, m_bladeAngle - angleMargin, m_prevBladeAngle + angleMargin)) {
                
                m_hitThisSwing.insert(e->getId());
                hits.push_back({e->getId(), AbilityType::VanguardKatanaSlash});
            }
        }
    }
    else if(m_isDashing){
        float hitRadius = getRadius() + 25.0f;
        for (Entity* e : entities) {
            if (e->getFaction() == Faction::Players) continue;
            if (m_hitDuringDash.count(e->getId())) continue;

            sf::Vector2f diff = e->getPosition() - m_position;
            if (diff.lengthSquared() < (hitRadius + e->getRadius()) * (hitRadius + e->getRadius())) {
                m_hitDuringDash.insert(e->getId());
                hits.push_back({e->getId(), AbilityType::VanguardDash});
            }
        }
    }

    return hits;
}


void Vanguard::render(sf::RenderTarget& target) {
    drawTrail(target);
    drawDashTrails(target);
    drawKatana(target);
    Player::render(target);
}

void Vanguard::updateTrail(float dt) {
    for (auto& frame : m_trail) frame.age += dt;

    m_trail.erase(std::remove_if(m_trail.begin(), m_trail.end(),
        [this](const SlashTrailFrame& f) { return f.age > m_swingDuration; }), m_trail.end());
}

void Vanguard::drawTrail(sf::RenderTarget& target) const {
    if (m_trail.size() < 2) return;

    sf::VertexArray mesh(sf::PrimitiveType::Triangles);

    for (std::size_t i = 0; i + 1 < m_trail.size(); ++i) {
        float angleA = m_trail[i].angle;
        float angleB = m_trail[i + 1].angle;
        float age = m_trail[i + 1].age;

        for (int ring = 0; ring < m_radialSegments; ++ring) {
            float t0 = static_cast<float>(ring) / m_radialSegments;
            float t1 = static_cast<float>(ring + 1) / m_radialSegments;
            float midX = (t0 + t1) * 0.5f;

            float lifetime = m_swingDuration * std::exp(2.0f * midX - 2.0f);
            if (lifetime <= 0.0f || age >= lifetime) continue;

            float spatial = std::exp(2.0f * midX - 2.0f);
            float alphaProgress = spatial * (1.0f - age / lifetime);
            if (alphaProgress <= 0.01f) continue;

            std::uint8_t a = static_cast<std::uint8_t>(255.0f * std::clamp(alphaProgress, 0.0f, 1.0f));
            sf::Color core(100, 255, 255, a);

            sf::Vector2f pA0 = getBladePoint(angleA, t0);
            sf::Vector2f pA1 = getBladePoint(angleA, t1);
            sf::Vector2f pB0 = getBladePoint(angleB, t0);
            sf::Vector2f pB1 = getBladePoint(angleB, t1);

            mesh.append(sf::Vertex{pA0, core}); mesh.append(sf::Vertex{pB0, core}); mesh.append(sf::Vertex{pB1, core});
            mesh.append(sf::Vertex{pA0, core}); mesh.append(sf::Vertex{pB1, core}); mesh.append(sf::Vertex{pA1, core});
        }
    }

    sf::RenderStates addBlend;
    addBlend.blendMode = sf::BlendAdd;
    if (mesh.getVertexCount() > 0) target.draw(mesh, addBlend);
}

void Vanguard::drawKatana(sf::RenderTarget& target) const {
    if (!m_attackActive) return;

    sf::Vector2f hilt = getBladePoint(m_bladeAngle, 0.0f);
    sf::Vector2f tip = getBladePoint(m_bladeAngle, 1.0f);
    sf::Vector2f dir = tip - hilt;
    float lenSq = dir.lengthSquared();
    if (lenSq < 0.001f) return;
    dir /= std::sqrt(lenSq);

    float halfW = m_katanaWidth * 0.5f;
    sf::Vector2f n{-dir.y * halfW, dir.x * halfW};

    sf::VertexArray body(sf::PrimitiveType::TriangleStrip, 4);
    body[0] = {hilt - n, sf::Color(220, 230, 255, 255)};
    body[1] = {hilt + n, sf::Color(220, 230, 255, 255)};
    body[2] = {tip + n, sf::Color::White};
    body[3] = {tip - n, sf::Color::White};
    
    target.draw(body);
}

float Vanguard::getBladeAngleAt(float progress) const {
    float base = std::atan2(m_aimDir.y, m_aimDir.x);
    float left = base - m_halfArcRad;
    float right = base + m_halfArcRad;
    return m_swingRight ? left + (right - left) * progress : right + (left - right) * progress;
}

sf::Vector2f Vanguard::getBladePoint(float angle, float radiusProgress) const {
    float radius = m_innerRadius + (m_outerRadius - m_innerRadius) * radiusProgress;
    return m_position + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
}

float Vanguard::normalizeAngle(float angle) const {
    return std::remainder(angle, 2.0f * M_PI);
}

bool Vanguard::isAngleBetween(float target, float angle1, float angle2) const {
    float diff1 = normalizeAngle(target - angle1);
    float diff2 = normalizeAngle(angle2 - angle1);
    
    if (diff2 < 0.0f) {
        diff1 = normalizeAngle(target - angle2);
        diff2 = -diff2;
    }
    return diff1 >= 0.0f && diff1 <= diff2;
}

void Vanguard::drawDashTrails(sf::RenderTarget& target) const {
    sf::RenderStates addBlend;
    addBlend.blendMode = sf::BlendAdd;

    for (const auto& trail : m_dashTrails) {
        float progress = trail.age / 0.35f;
        float fade = std::max(0.0f, 1.0f - progress);
        
        std::uint8_t alpha = static_cast<std::uint8_t>(255.0f * fade);
        sf::Color edgeColor(0, 255, 255, alpha);
        sf::Color coreColor(255, 255, 255, alpha);

        sf::Vector2f dir = trail.end - trail.start;
        float lenSq = dir.lengthSquared();
        if (lenSq < 1.0f) continue;
        sf::Vector2f dirNorm = dir / std::sqrt(lenSq);

        float currentWidth = 14.0f * fade; 
        sf::Vector2f n(-dirNorm.y * currentWidth, dirNorm.x * currentWidth);

        sf::VertexArray wedge(sf::PrimitiveType::Triangles, 6);
        
        wedge[0] = sf::Vertex(trail.start, sf::Color(0, 255, 255, 0));
        wedge[1] = sf::Vertex(trail.end - n, edgeColor);              
        wedge[2] = sf::Vertex(trail.end, coreColor);                   

        wedge[3] = sf::Vertex(trail.start, sf::Color(0, 255, 255, 0)); 
        wedge[4] = sf::Vertex(trail.end, coreColor);
        wedge[5] = sf::Vertex(trail.end + n, edgeColor);

        target.draw(wedge, addBlend);
    }
}

void Vanguard::onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (m_dashCharges > 0 && !m_isDashing) {
        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();
        if (lenSq > 0.0f) m_dashDir = dir / std::sqrt(lenSq);
        else m_dashDir = sf::Vector2f(1.0f, 0.0f);

        m_isDashing = true;
        m_dashCharges--;
        m_dashDistanceRemaining = 220.0f;
        
        m_hitDuringDash.clear();
        m_dashStartPos = m_position;
        m_isFocused = false;

        if (engine.getServerAddress()) {
            sf::Packet packet;
            packet << PacketType::AbilityUsed << m_id << AbilityType::VanguardDash << m_dashDir;
            (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

void Vanguard::onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (!m_isUltActive && m_ultCharge >= m_maxUltCharge) {
        m_isUltActive = true;
        m_ultTimer = 8.0f;
        m_ultCharge = 0.0f; 
        
        m_speed *= 2.0f;
    }
}

void Vanguard::onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    (void)mouseWorldPos; (void)projMgr;

    if (m_cooldownE > 0.0f || getStealthTimer() > 0.0f) return;

    m_cooldownE = m_maxCooldownE;
    setStealthTimer(Config::VANGUARD_STEALTH_DURATION);

    if (engine.getServerAddress()) {
        sf::Packet packet;
        packet << PacketType::AbilityUsed << m_id << AbilityType::VanguardDecoy << m_position;
        (void)engine.getSocket().send(packet, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

void Vanguard::onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {
    if (m_cooldownRMB <= 0.0f && m_shurikensToFire == 0) {
        m_cooldownRMB = m_maxCooldownRMB;
        
        m_shurikensToFire = 5;
        m_shurikenBurstTimer = 0.0f;
        
        sf::Vector2f dir = mouseWorldPos - m_position;
        float lenSq = dir.lengthSquared();
        if (lenSq > 0.0001f) {
            m_shurikenAimDir = dir / std::sqrt(lenSq);
        } else {
            m_shurikenAimDir = sf::Vector2f(1.0f, 0.0f);
        }
        
        m_engineRef = &engine;
        m_projMgrRef = &projMgr;
    }
}

void Vanguard::onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) {
    if (m_attackActive || m_cooldownLMB > 0.0f) return;

    sf::Vector2f dir = mouseWorldPos - m_position;
    float lenSq = dir.lengthSquared();
    m_aimDir = (lenSq > 0.0f) ? (dir / std::sqrt(lenSq)) : sf::Vector2f(1.0f, 0.0f);

    m_swingRight = !m_swingRight;
    m_trail.clear();
    m_hitThisSwing.clear();
    m_attackTime = 0.0f;
    
    m_bladeAngle = getBladeAngleAt(0.0f);
    m_prevBladeAngle = m_bladeAngle;
    m_attackActive = true;

    if (m_isUltActive && engine.getServerAddress()) {
        WeaponType weapon = WeaponType::VanguardWave;
        
        sf::Vector2f waveTarget = m_position + m_aimDir * 1000.0f;
        projMgr.spawnProjectile(m_id, m_position, waveTarget, weapon, Faction::Players);
        
        sf::Packet shootPacket;
        shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << waveTarget;
        (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
    }
}

void Vanguard::renderShiftSkill() {
    ImGui::Text("SHIFT");
    
    for (int i = 0; i < m_maxDashCharges; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 4.0f);
        
        ImGui::PushID(i);
        
        if (i < m_dashCharges) {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 1.0f, 1.0f, 1.0f)); 
            ImGui::ProgressBar(1.0f, ImVec2(25.0f, 15.0f), "");
        } 
        else if (i == m_dashCharges) {
            float progress = m_dashRechargeTimer / 2.5f;
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.5f, 0.6f, 1.0f)); 
            ImGui::ProgressBar(progress, ImVec2(25.0f, 15.0f), "");
        } 
        else {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); 
            ImGui::ProgressBar(0.0f, ImVec2(25.0f, 15.0f), "");
        }
        
        ImGui::PopStyleColor();
        ImGui::PopID();
    }
}

void Vanguard::renderESkill() {
    ImGui::Text("E");

    if (getStealthTimer() > 0.0f) {
        float progress = getStealthTimer() / Config::VANGUARD_STEALTH_DURATION;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), "STEALTH");
        ImGui::PopStyleColor();
    } else if (m_cooldownE <= 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        float progress = 1.0f - (m_cooldownE / m_maxCooldownE);
        char cdText[16];
        sprintf(cdText, "%.1fs", m_cooldownE);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

void Vanguard::renderRightPanel() {
    ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH - 320.0f, Config::WINDOW_HEIGHT - 80.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 100.0f), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("HeroStatsRight", nullptr, flags);

    // HP
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "HP: %.0f / %.0f", m_hp, m_maxHp);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
    ImGui::ProgressBar(m_hp / m_maxHp, ImVec2(-1, 15.0f), "");
    ImGui::PopStyleColor();

    ImGui::End();
}