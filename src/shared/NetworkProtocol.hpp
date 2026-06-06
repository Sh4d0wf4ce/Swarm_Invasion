#pragma once

#include <SFML/Network.hpp>

enum class PacketType : std::uint8_t{
    Ping,
    Pong,
    PlayerPosition,
    WorldState,
    JoinRequest,
    JoinAccept,
    PlayerShoots,
    PlayerDied,
    PlayerDisconnect,
    LevelUpTriggered,
    LevelUpOffer,
    UpgradeChosen,
    UpgradeResolved,
    CardSelected,
    PlayerUpgradeMultipliers,
    EnemyShoots,
    EntityHit,
    SpawnHealField,
    SpawnBlackHole,
    SpawnDecoy,
    SpawnMedicOrb,
    SpawnMedicBarrier,
    SpawnMedicDrone,
    DroneShoots,
    DecoyExplode,
    PlayerDealtDamage,
    AbilityUsed,
    AbilityHit
};

enum class PlayerClass : std::uint8_t{
    Soldier,
    Medic,
    Juggernaut,
    Vanguard
};

enum class EnemyType : std::uint8_t{
    Crawler,
    Bruiser,
    Spitter,
    Kamikaze
};

enum class WeaponType : std::uint8_t{
    None,
    Rifle,
    Laser,
    Rocket,
    AcidSpit,
    Shotgun,
    Shuriken,
    VanguardWave,
    MedicNeedle,
    DroneBlaster,
};

enum class AbilityType : std::uint8_t{
    JuggernautDash,
    JuggernautBlackHole,
    JuggernautRepulsor,
    SoldierHealField,
    VanguardKatanaSlash,
    VanguardDash,
    VanguardDecoy,
    VanguardDecoyExplode,
    MedicTeleport,
    MedicOrb,
    MedicBarrier,
    MedicUltCommand
};

enum class MedicDroneState : std::uint8_t {
    Orbit,
    Sentry
};

inline sf::Packet& operator<<(sf::Packet& packet, const MedicDroneState& state){
    return packet << static_cast<std::uint8_t>(state);
}

inline sf::Packet& operator>>(sf::Packet& packet, MedicDroneState& state){
    std::uint8_t value;
    packet >> value;
    state = static_cast<MedicDroneState>(value);
    return packet;
}

inline sf::Packet& operator<<(sf::Packet& packet, const AbilityType& aType){
    return packet << static_cast<std::uint8_t>(aType);
}

inline sf::Packet& operator>>(sf::Packet& packet, AbilityType& aType){
    std::uint8_t value;
    packet >> value;
    aType = static_cast<AbilityType>(value);
    return packet;
}

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

