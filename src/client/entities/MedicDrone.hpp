#pragma once
#include "Entity.hpp"
#include "AbilityRegistry.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

/**
 * @brief Medic ultimate drone with orbiting nodes and a spinning energy ring.
 *
 * Interpolates toward server position, pulses its core shape, and renders
 * three orbiting nodes. Belongs to no combat faction and has zero radius.
 */
class MedicDrone : public Entity {
public:

    /**
     * @brief Constructs a drone owned by a Medic player.
     * @param id Unique network entity identifier.
     * @param ownerId Network ID of the Medic that deployed the drone.
     * @param startPos Initial world position.
     */
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

    /**
     * @brief Interpolates position, pulses the core, and rebuilds the ring mesh.
     * @param deltaTime Elapsed time since the last frame.
     * @param map Tile map reference.
     */
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

    /**
     * @brief Draws the core, spinning ring, and three orbiting nodes.
     * @param target Render target to draw into.
     */
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

    /**
     * @brief Sets server position for interpolation.
     * @param pos Latest world position from the server.
     */
    void setServerPosition(const sf::Vector2f& pos) { m_serverPosition = pos; }

    /**
     * @brief Updates the drone behavioral state from network sync.
     * @param state Current MedicDroneState (orbit, heal, etc.).
     */
    void setDroneState(MedicDroneState state) { m_state = state; }

    /**
     * @brief Returns the owning Medic player's network ID.
     * @return Owner entity ID.
     */
    std::uint32_t getOwnerId() const { return m_ownerId; }

    /**
     * @brief Returns the current drone behavioral state.
     * @return Active MedicDroneState value.
     */
    MedicDroneState getDroneState() const { return m_state; }

    /**
     * @brief Drones are neutral and do not belong to a combat faction.
     * @return Faction::None.
     */
    Faction getFaction() const override { return Faction::None; }

    /**
     * @brief Drones use visual shapes rather than a circular collision radius.
     * @return 0.0f.
     */
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
    /**
     * @brief Rebuilds the spinning ring vertex array around the drone core.
     */
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
