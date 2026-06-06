#pragma once

#include "Entity.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

class MedicBarrier : public Entity {
public:
    MedicBarrier(std::uint32_t id, const sf::Vector2f& arcCenter, float facingAngle, float duration)
        : Entity(id, arcCenter), m_center(arcCenter), m_facingAngle(facingAngle) {
        m_hp = duration;
        m_maxHp = duration;
        m_position = arcCenter;
        buildArcMesh();
    }

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
        (void)map;
        m_hp -= deltaTime.asSeconds();
        m_animTime += deltaTime.asSeconds();
    }

    void render(sf::RenderTarget& target) override {
        float pulse = 0.5f + 0.5f * std::sin(m_animTime * 6.0f);
        const std::uint8_t alpha = static_cast<std::uint8_t>(100 + 80 * pulse);

        for (std::size_t i = 0; i < m_fillVertices.getVertexCount(); ++i) {
            m_fillVertices[i].color.a = alpha;
        }
        for (std::size_t i = 0; i < m_outlineVertices.getVertexCount(); ++i) {
            m_outlineVertices[i].color.a = static_cast<std::uint8_t>(std::min(255, static_cast<int>(alpha) + 60));
        }

        target.draw(m_fillVertices);
        target.draw(m_outlineVertices);
    }

    Faction getFaction() const override { return Faction::None; }
    float getRadius() const override { return 0.0f; }

    sf::Vector2f getCenter() const { return m_center; }
    float getFacingAngle() const { return m_facingAngle; }

private:
    sf::Vector2f m_center;
    float m_facingAngle;
    float m_animTime{0.0f};
    sf::VertexArray m_fillVertices;
    sf::VertexArray m_outlineVertices;

    void buildArcMesh() {
        constexpr int segments = 36;
        const float halfSpan = Config::MEDIC_BARRIER_SPAN / 2.0f;
        const float innerR = Config::MEDIC_BARRIER_ARC_RADIUS - Config::MEDIC_BARRIER_WALL_THICKNESS / 2.0f;
        const float outerR = Config::MEDIC_BARRIER_ARC_RADIUS + Config::MEDIC_BARRIER_WALL_THICKNESS / 2.0f;
        const float startAngle = m_facingAngle - halfSpan;
        const float endAngle = m_facingAngle + halfSpan;

        m_fillVertices.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
        m_fillVertices.resize(static_cast<std::size_t>((segments + 1) * 2));

        for (int i = 0; i <= segments; ++i) {
            const float t = static_cast<float>(i) / segments;
            const float angle = startAngle + t * (endAngle - startAngle);
            const float cosA = std::cos(angle);
            const float sinA = std::sin(angle);

            sf::Vector2f inner(m_center.x + cosA * innerR, m_center.y + sinA * innerR);
            sf::Vector2f outer(m_center.x + cosA * outerR, m_center.y + sinA * outerR);

            m_fillVertices[static_cast<std::size_t>(i * 2)] = sf::Vertex(inner, sf::Color(50, 255, 200, 180));
            m_fillVertices[static_cast<std::size_t>(i * 2 + 1)] = sf::Vertex(outer, sf::Color(100, 255, 255, 180));
        }

        m_outlineVertices.setPrimitiveType(sf::PrimitiveType::LineStrip);
        m_outlineVertices.resize(static_cast<std::size_t>(segments + 1));
        for (int i = 0; i <= segments; ++i) {
            const float t = static_cast<float>(i) / segments;
            const float angle = startAngle + t * (endAngle - startAngle);
            sf::Vector2f outer(
                m_center.x + std::cos(angle) * outerR,
                m_center.y + std::sin(angle) * outerR
            );
            m_outlineVertices[static_cast<std::size_t>(i)] = sf::Vertex(outer, sf::Color(100, 255, 255, 255));
        }
    }
};
