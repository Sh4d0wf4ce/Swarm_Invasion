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
    PlayerDied,
    PlayerDisconnect,
    LevelUpTriggered,
    CardSelected,
    EnemyShoots,
    PlayerHit
};

enum class PlayerClass : std::uint8_t{
    Soldier,
    Scout,
    Tank,
    Medic
};

enum class EnemyType : std::uint8_t{
    Crawler,
    Bruiser,
    Spitter
};

enum class WeaponType : std::uint8_t{
    Rifle,
    Laser,
    Rocket,
    AcidSpit
};

inline sf::Packet& operator<<(sf::Packet& packet, const WeaponType& wType){
    return packet << static_cast<std::uint8_t>(wType);
}

inline sf::Packet& operator>>(sf::Packet& packet, WeaponType& wType){
    std::uint8_t value;
    packet >> value;
    wType = static_cast<WeaponType>(value);
    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const EnemyType& eType){
    return packet << static_cast<std::uint8_t>(eType);
}

inline sf::Packet& operator>>(sf::Packet& packet, EnemyType& eType){
    std::uint8_t value;
    packet >> value;
    eType = static_cast<EnemyType>(value);
    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, PacketType type){
    return packet << static_cast<std::uint8_t>(type);
}

inline sf::Packet& operator>>(sf::Packet& packet, PacketType& type){
    std::uint8_t value;
    packet >> value;
    type = static_cast<PacketType>(value);
    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& vector){
    return packet << vector.x << vector.y;
}

inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& vector){
    return packet >> vector.x >> vector.y;
}

inline sf::Packet& operator<<(sf::Packet& packet, const PlayerClass& pClass){
    return packet << static_cast<std::uint8_t>(pClass);
}

inline sf::Packet& operator>>(sf::Packet& packet, PlayerClass& pClass){
    std::uint8_t value;
    packet >> value;
    pClass = static_cast<PlayerClass>(value);
    return packet;
}

