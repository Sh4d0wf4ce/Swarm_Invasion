/**
 * @file NetworkProtocol.hpp
 * @brief Shared network packet types, game entity enums, and SFML packet serialization operators.
 */
#pragma once

#include <SFML/Network.hpp>

// ==========================================
// Network Packet Types
// ==========================================
/**
 * @brief Identifies the payload type of a network packet exchanged between client and server.
 */
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

// ==========================================
// Game Entity Enums
// ==========================================
/**
 * @brief Playable hero classes available in multiplayer sessions.
 */
enum class PlayerClass : std::uint8_t{
    Soldier,
    Medic,
    Juggernaut,
    Vanguard
};

/**
 * @brief Enemy archetypes spawned during invasion waves.
 */
enum class EnemyType : std::uint8_t{
    Crawler,
    Bruiser,
    Spitter,
    Kamikaze
};

/**
 * @brief Projectile and weapon identifiers used by players, enemies, and drones.
 */
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

/**
 * @brief Active ability identifiers mapped to hero-specific skills.
 */
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

/**
 * @brief Behavioral modes for the Medic's companion drone.
 */
enum class MedicDroneState : std::uint8_t {
    Orbit,
    Sentry
};

// ==========================================
// Packet Serialization Operators
// ==========================================
/**
 * @brief Serializes a MedicDroneState value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param state Drone state to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const MedicDroneState& state){
    return packet << static_cast<std::uint8_t>(state);
}

/**
 * @brief Deserializes a MedicDroneState value from an SFML packet.
 * @param packet Source packet buffer.
 * @param state Output drone state.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, MedicDroneState& state){
    std::uint8_t value;
    packet >> value;
    state = static_cast<MedicDroneState>(value);
    return packet;
}

/**
 * @brief Serializes an AbilityType value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param aType Ability type to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const AbilityType& aType){
    return packet << static_cast<std::uint8_t>(aType);
}

/**
 * @brief Deserializes an AbilityType value from an SFML packet.
 * @param packet Source packet buffer.
 * @param aType Output ability type.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, AbilityType& aType){
    std::uint8_t value;
    packet >> value;
    aType = static_cast<AbilityType>(value);
    return packet;
}

/**
 * @brief Serializes a WeaponType value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param wType Weapon type to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const WeaponType& wType){
    return packet << static_cast<std::uint8_t>(wType);
}

/**
 * @brief Deserializes a WeaponType value from an SFML packet.
 * @param packet Source packet buffer.
 * @param wType Output weapon type.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, WeaponType& wType){
    std::uint8_t value;
    packet >> value;
    wType = static_cast<WeaponType>(value);
    return packet;
}

/**
 * @brief Serializes an EnemyType value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param eType Enemy type to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const EnemyType& eType){
    return packet << static_cast<std::uint8_t>(eType);
}

/**
 * @brief Deserializes an EnemyType value from an SFML packet.
 * @param packet Source packet buffer.
 * @param eType Output enemy type.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, EnemyType& eType){
    std::uint8_t value;
    packet >> value;
    eType = static_cast<EnemyType>(value);
    return packet;
}

/**
 * @brief Serializes a PacketType value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param type Packet type to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, PacketType type){
    return packet << static_cast<std::uint8_t>(type);
}

/**
 * @brief Deserializes a PacketType value from an SFML packet.
 * @param packet Source packet buffer.
 * @param type Output packet type.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, PacketType& type){
    std::uint8_t value;
    packet >> value;
    type = static_cast<PacketType>(value);
    return packet;
}

/**
 * @brief Serializes a 2D vector into an SFML packet as two floats.
 * @param packet Destination packet buffer.
 * @param vector Vector to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& vector){
    return packet << vector.x << vector.y;
}

/**
 * @brief Deserializes a 2D vector from an SFML packet.
 * @param packet Source packet buffer.
 * @param vector Output vector.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& vector){
    return packet >> vector.x >> vector.y;
}

/**
 * @brief Serializes a PlayerClass value into an SFML packet as a single byte.
 * @param packet Destination packet buffer.
 * @param pClass Player class to write.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator<<(sf::Packet& packet, const PlayerClass& pClass){
    return packet << static_cast<std::uint8_t>(pClass);
}

/**
 * @brief Deserializes a PlayerClass value from an SFML packet.
 * @param packet Source packet buffer.
 * @param pClass Output player class.
 * @return Reference to @p packet for chaining.
 */
inline sf::Packet& operator>>(sf::Packet& packet, PlayerClass& pClass){
    std::uint8_t value;
    packet >> value;
    pClass = static_cast<PlayerClass>(value);
    return packet;
}
