#pragma once
#include "Entity.hpp"
#include "AbilityRegistry.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

class MedicDrone : public Entity {
public:

    MedicDrone(std::uint32_t id, std::uint32_t ownerId, const sf::Vector2f& startPos)
        : Entity(id, startPos), m_ownerId(ownerId), m_serverPosition(startPos) {
        m_hp = AbilityRegistry::medic().Drone.lifetime;
        m_maxHp = AbilityRegistry::medic().Drone.lifetime;
        m_position = startPos;
        m_coreShape.setRadius(10.0f);
        m_coreShape.setOrigin({10.0f, 10.0f});
        m_coreShape.setFillColor(sf::Color(80, 220, 255, 140));
        m_coreShape.setOutlineThickness(2.0f);
        m_coreShape.setOutlineColor(sf::Color(180, 255, 255, 200));
    }

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
        float dt = deltaTime.asSeconds();

        m_animTime += dt;
        m_position += (m_serverPosition - m_position) * std::min(1.0f, dt * 8.0f);
        const float pulse = 0.5f + 0.5f * std::sin(m_animTime * 5.0f);

        m_coreShape.setRadius(10.0f + 1.5f * pulse);
        m_coreShape.setOrigin({m_coreShape.getRadius(), m_coreShape.getRadius()});
        m_coreShape.setPosition(m_position);
        m_coreShape.setFillColor(sf::Color(80, 220, 255, 140));
        m_coreShape.setOutlineThickness(2.0f);
        m_coreShape.setOutlineColor(sf::Color(180, 255, 255, static_cast<std::uint8_t>(180 + 60 * pulse)));
        buildRingVertices();
    }

    void render(sf::RenderTarget& target) override {
        target.draw(m_coreShape);
        target.draw(m_ringVertices);

        for (int i = 0; i < 3; ++i) {
            float angle = m_animTime * (2.0f + i * 0.7f) + i * 2.094f;
            float orbitR = 18.0f + i * 4.0f;
            sf::CircleShape node(3.0f + i * 0.5f);
            node.setOrigin({node.getRadius(), node.getRadius()});
            node.setPosition(m_position + sf::Vector2f(std::cos(angle) * orbitR, std::sin(angle) * orbitR));
            node.setFillColor(sf::Color(100, 255, 220, static_cast<std::uint8_t>(160 + 40 * i)));
            target.draw(node);
        }
    }

    void setServerPosition(const sf::Vector2f& pos) { m_serverPosition = pos; }
    void setDroneState(MedicDroneState state) { m_state = state; }
    std::uint32_t getOwnerId() const { return m_ownerId; }
    MedicDroneState getDroneState() const { return m_state; }

    Faction getFaction() const override { return Faction::None; }
    float getRadius() const override { return 0.0f; }
private:

    // Ownership & Sync State
    std::uint32_t m_ownerId;
    sf::Vector2f m_serverPosition;
    MedicDroneState m_state{MedicDroneState::Orbit};


    // Animation State
    float m_animTime{0.0f};

    // Visual Shapes
    sf::CircleShape m_coreShape;
    sf::VertexArray m_ringVertices{sf::PrimitiveType::LineStrip};


    // Mesh Construction
    void buildRingVertices() {
        constexpr int segments = 24;
        m_ringVertices.resize(static_cast<std::size_t>(segments + 1));
        const float ringR = 16.0f;
        for (int i = 0; i <= segments; ++i) {
            float angle = m_animTime * 1.5f + (static_cast<float>(i) / segments) * 2 * M_PI;
            sf::Vector2f pt(
                m_position.x + std::cos(angle) * ringR,
                m_position.y + std::sin(angle) * ringR
            );
            m_ringVertices[static_cast<std::size_t>(i)] = sf::Vertex(pt, sf::Color(0, 200, 255, 120));
        }
    }
};
