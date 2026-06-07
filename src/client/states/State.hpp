#pragma once 

#include "NetworkProtocol.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

class ClientEngine;

/**
 * @brief Abstract base class for client application states.
 *
 * Each state implements the game loop hooks invoked by ClientEngine:
 * packet handling, input, simulation update, world rendering, and UI.
 * States receive a reference to the owning engine for window and network access.
 */
class State{
public:
    /**
     * @brief Constructs a state bound to the given client engine.
     * @param engine Client engine that owns and drives this state.
     */
    State(ClientEngine& engine) : m_engine(engine) {}

    /** @brief Virtual destructor for polymorphic state cleanup. */
    virtual ~State() = default;

    /** @brief Called when this state becomes active after a state change. */
    virtual void onEnter() {}

    /** @brief Called when this state is about to be replaced or destroyed. */
    virtual void onExit() {}


    // Interface
    virtual void handlePacket(PacketType type, sf::Packet& packet) = 0;
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

    /** @brief Renders ImGui overlays; default implementation draws nothing. */
    virtual void renderUI() {};

protected:
    ClientEngine& m_engine;
};
