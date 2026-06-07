#pragma once
#include "Entity.hpp"
#include "AbilityRegistry.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

/**
 * @brief Medic directional arc barrier rendered as a pulsing wall mesh.
 *
 * Builds fill and outline vertex arrays from registry geometry parameters.
 * Uses remaining lifetime as HP and has no faction or collision radius.
 */
class MedicBarrier : public Entity {
public:
    // ==========================================
    // Construction
    // ==========================================
    /**
     * @brief Constructs a barrier arc centered on the medic facing a direction.
     * @param id Unique network entity identifier.
     * @param arcCenter Center of the arc in world space.
     * @param facingAngle Facing direction in radians.
     * @param duration Total lifetime in seconds stored as HP.
     */
    MedicBarrier(std::uint32_t id, const sf::Vector2f& arcCenter, float facingAngle, float duration)
        : Entity(id, arcCenter), m_center(arcCenter), m_facingAngle(facingAngle) {
        m_hp = duration;
        m_maxHp = duration;
        m_position = arcCenter;
        buildArcMesh();
    }

    // ==========================================
    // Entity Lifecycle
    // ==========================================
    /**
     * @brief Reduces remaining lifetime and advances pulse animation time.
     * @param deltaTime Elapsed time since the last frame.
     * @param map Tile map reference.
     */
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
        (void)map;
        m_hp -= deltaTime.asSeconds();
        m_animTime += deltaTime.asSeconds();
    }

    /**
     * @brief Draws the pulsing fill and outline arc meshes.
     * @param target Render target to draw into.
     */
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

    // ==========================================
    // Identity & Collision
    // ==========================================
    /**
     * @brief Barriers are neutral and do not belong to a combat faction.
     * @return Faction::None.
     */
    Faction getFaction() const override { return Faction::None; }

    /**
     * @brief Barriers use mesh geometry rather than a circular radius.
     * @return 0.0f.
     */
    float getRadius() const override { return 0.0f; }

    // ==========================================
    // Geometry Accessors
    // ==========================================
    /**
     * @brief Returns the arc center in world space.
     * @return Center position of the barrier arc.
     */
    sf::Vector2f getCenter() const { return m_center; }

    /**
     * @brief Returns the facing angle of the barrier arc.
     * @return Facing direction in radians.
     */
    float getFacingAngle() const { return m_facingAngle; }
private:
    // ==========================================
    // Geometry & Animation State
    // ==========================================
    sf::Vector2f m_center;
    float m_facingAngle;
    float m_animTime{0.0f};

    // ==========================================
    // Visual Meshes
    // ==========================================
    sf::VertexArray m_fillVertices;
    sf::VertexArray m_outlineVertices;

    // ==========================================
    // Mesh Construction
    // ==========================================
    /**
     * @brief Builds fill and outline vertex arrays from registry parameters.
     */
    void buildArcMesh() {
        constexpr int segments = 36;
        const auto& barrier = AbilityRegistry::medic().Barrier;
        const float span = AbilityRegistry::param(barrier, "span", 0.80f);
        const float arcRadius = AbilityRegistry::param(barrier, "arcRadius", 280.f);
        const float wallThickness = AbilityRegistry::param(barrier, "wallThickness", 18.f);
        const float halfSpan = span / 2.0f;
        const float innerR = arcRadius - wallThickness / 2.0f;
        const float outerR = arcRadius + wallThickness / 2.0f;
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
