#pragma once

#include <SFML/Network.hpp>

enum class PacketType : std::uint8_t{
    Ping,
    Pong,
    PlayerPosition,
    WorldState,
    EnemyHit,
    JoinRequest,
    JoinAccept,
    PlayerShoots,
    PlayerDied
};

inline sf::Packet& operator<<(sf::Packet& packet, PacketType type){
    return packet << static_cast<std::uint8_t>(type);
}

inline sf::Packet& operator>>(sf::Packet& packet, PacketType& type){
    std::uint8_t value;
    packet>>value;
    type = static_cast<PacketType>(value);
    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& vector){
    return packet << vector.x << vector.y;
}

inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& vector){
    return packet >> vector.x >> vector.y;
}