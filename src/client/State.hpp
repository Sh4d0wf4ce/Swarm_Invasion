#pragma once 

#include "NetworkProtocol.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

class ClientEngine;

class State{
public:
    State(ClientEngine& engine) : m_engine(engine) {}
    virtual ~State() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void handlePacket(PacketType type, sf::Packet& packet) = 0;
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;
    virtual void renderUI() {};

protected:
    ClientEngine& m_engine;
};