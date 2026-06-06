#pragma once

#include "Entity.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <vector>

class MedicOrb : public Entity {
public:
    MedicOrb(std::uint32_t id, const sf::Vector2f& startPos, const sf::Vector2f& direction)
        : Entity(id, startPos) {
        m_hp = Config::MEDIC_ORB_LIFETIME;
        m_maxHp = Config::MEDIC_ORB_LIFETIME;
        m_serverPosition = startPos;

        float lenSq = direction.lengthSquared();
        if (lenSq < 1e-4f) {
            m_velocity = sf::Vector2f(Config::MEDIC_ORB_SPEED, 0.0f);
        } else {
            m_velocity = (direction / std::sqrt(lenSq)) * Config::MEDIC_ORB_SPEED;
        }

        m_innerShape.setRadius(12.0f);
        m_innerShape.setOrigin({12.0f, 12.0f});
        m_innerShape.setFillColor(sf::Color(50, 255, 120, 180));

        m_outerShape.setRadius(18.0f);
        m_outerShape.setOrigin({18.0f, 18.0f});
        m_outerShape.setFillColor(sf::Color::Transparent);
        m_outerShape.setOutlineThickness(2.0f);
        m_outerShape.setOutlineColor(sf::Color(180, 50, 255, 200));
    }

    void setServerPosition(const sf::Vector2f& pos) {
        m_serverPosition = pos;
    }

    void setNearbyEntities(const std::vector<Entity*>& allies, const std::vector<Entity*>& enemies) {
        m_allies = allies;
        m_enemies = enemies;
    }

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
        float dt = deltaTime.asSeconds();
        m_hp -= dt;
        m_animTime += dt;

        sf::Vector2f velocity = m_velocity * dt;

        sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
        if (map && map->checkCollision(nextPosX, Config::MEDIC_ORB_RADIUS)) {
            m_velocity.x = -m_velocity.x;
        } else {
            m_position.x = nextPosX.x;
        }

        sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
        if (map && map->checkCollision(nextPosY, Config::MEDIC_ORB_RADIUS)) {
            m_velocity.y = -m_velocity.y;
        } else {
            m_position.y = nextPosY.y;
        }

        m_position += (m_serverPosition - m_position) * 0.3f;

        float pulse = 0.5f + 0.5f * std::sin(m_animTime * 8.0f);
        m_outerShape.setOutlineThickness(2.0f + 2.0f * pulse);
        m_outerShape.setOutlineColor(sf::Color(180, 50, 255, static_cast<std::uint8_t>(150 + 80 * pulse)));

        m_innerShape.setPosition(m_position);
        m_outerShape.setPosition(m_position);
    }

    void render(sf::RenderTarget& target) override {
        const float effectRadiusSq = Config::MEDIC_ORB_EFFECT_RADIUS * Config::MEDIC_ORB_EFFECT_RADIUS;

        for (Entity* ally : m_allies) {
            if (!ally) continue;
            float distSq = (ally->getPosition() - m_position).lengthSquared();
            if (distSq <= effectRadiusSq) {
                drawTether(target, m_position, ally->getPosition(), sf::Color(0, 255, 100, 200));
            }
        }

        for (Entity* enemy : m_enemies) {
            if (!enemy) continue;
            float distSq = (enemy->getPosition() - m_position).lengthSquared();
            if (distSq <= effectRadiusSq) {
                drawTether(target, m_position, enemy->getPosition(), sf::Color(200, 50, 255, 200));
            }
        }

        target.draw(m_outerShape);
        target.draw(m_innerShape);
    }

    Faction getFaction() const override { return Faction::Players; }
    float getRadius() const override { return Config::MEDIC_ORB_RADIUS; }

private:
    static void drawTether(sf::RenderTarget& target, sf::Vector2f from, sf::Vector2f to, sf::Color color) {
        constexpr int segments = 6;
        sf::Vector2f delta = to - from;
        float lenSq = delta.lengthSquared();
        if (lenSq < 1.0f) return;

        sf::Vector2f step = delta / static_cast<float>(segments);
        sf::Vector2f perp(-step.y, step.x);
        float perpLen = std::sqrt(perp.x * perp.x + perp.y * perp.y);
        if (perpLen > 0.0f) perp /= perpLen;

        sf::VertexArray line(sf::PrimitiveType::LineStrip, segments + 1);
        line[0] = sf::Vertex(from, color);

        for (int i = 1; i < segments; ++i) {
            float jitter = ((std::rand() % 100) / 100.0f - 0.5f) * 24.0f;
            sf::Vector2f point = from + step * static_cast<float>(i) + perp * jitter;
            line[i] = sf::Vertex(point, color);
        }

        line[segments] = sf::Vertex(to, color);
        target.draw(line);
    }

    sf::Vector2f m_velocity;
    sf::Vector2f m_serverPosition;
    float m_animTime{0.0f};

    std::vector<Entity*> m_allies;
    std::vector<Entity*> m_enemies;

    sf::CircleShape m_innerShape;
    sf::CircleShape m_outerShape;
};
