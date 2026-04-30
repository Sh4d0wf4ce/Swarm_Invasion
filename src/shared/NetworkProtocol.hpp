#pragma once

#include <SFML/Network.hpp>

enum class PacketType : std::uint8_t{
    Ping,
    Pong,
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