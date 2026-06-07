#pragma once
#include "NetworkProtocol.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// ==========================================
// Ability Stat Definitions
// ==========================================
/**
 * @brief Numeric tuning data for a single ability loaded from JSON configuration.
 */
struct AbilityStats {
    float cooldown{0.f};
    float damage{0.f};
    float knockback{0.f};
    float radius{0.f};
    float range{0.f};
    float duration{0.f};
    float speed{0.f};
    float lifetime{0.f};
    float heal{0.f};
    std::unordered_map<std::string, float> customParams;
};

/**
 * @brief Grouped ability stats for all Vanguard hero skills.
 */
struct VanguardAbilities {
    AbilityStats Decoy;
    AbilityStats Dash;
    AbilityStats KatanaSlash;
    AbilityStats Ult;
    AbilityStats ShurikenBurst;
};

/**
 * @brief Grouped ability stats for all Medic hero skills.
 */
struct MedicAbilities {
    AbilityStats Teleport;
    AbilityStats Orb;
    AbilityStats Needle;
    AbilityStats Barrier;
    AbilityStats Passive;
    AbilityStats Drone;
};

/**
 * @brief Grouped ability stats for all Juggernaut hero skills.
 */
struct JuggernautAbilities {
    AbilityStats Dash;
    AbilityStats Repulsor;
    AbilityStats BlackHole;
    AbilityStats Ult;
};

/**
 * @brief Grouped ability stats for all Soldier hero skills.
 */
struct SoldierAbilities {
    AbilityStats HealField;
    AbilityStats Ult;
    AbilityStats Sprint;
    AbilityStats Rocket;
    AbilityStats AutoAim;
};

/**
 * @brief Central registry that loads and exposes hero ability tuning data from JSON files.
 */
class AbilityRegistry {
public:
    /**
     * @brief Loads ability configuration from a JSON file and populates all class registries.
     * @param filepath Path to the abilities JSON configuration file.
     */
    static void loadConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[REGISTRY ERROR] Could not open " << filepath << "!\n";
            return;
        }

        nlohmann::json json;
        try {
            file >> json;
        } catch (const std::exception& e) {
            std::cerr << "[REGISTRY ERROR] Failed to parse " << filepath << ": " << e.what() << "\n";
            return;
        }
        file.close();
        m_stats.clear();
        m_vanguard = {};
        m_medic = {};
        m_juggernaut = {};
        m_soldier = {};
        int abilityCount = 0;
        for (auto& [classKey, classVal] : json.items()) {
            if (!classVal.is_object()) continue;
            if (classKey == "Vanguard") {
                abilityCount += loadClass(classVal, m_vanguard);
            } else if (classKey == "Medic") {
                abilityCount += loadClass(classVal, m_medic);
            } else if (classKey == "Juggernaut") {
                abilityCount += loadClass(classVal, m_juggernaut);
            } else if (classKey == "Soldier") {
                abilityCount += loadClass(classVal, m_soldier);
            } else {
                std::cerr << "[REGISTRY WARNING] Unknown class key: " << classKey << "\n";
            }
        }

        std::cout << "[REGISTRY] Successfully loaded " << filepath
                  << " (4 classes, " << abilityCount << " abilities)\n";
    }

    /**
     * @brief Returns the loaded Vanguard ability stats bundle.
     * @return Reference to Vanguard ability stats.
     */
    static const VanguardAbilities& vanguard() { return m_vanguard; }

    /**
     * @brief Returns the loaded Medic ability stats bundle.
     * @return Reference to Medic ability stats.
     */
    static const MedicAbilities& medic() { return m_medic; }

    /**
     * @brief Returns the loaded Juggernaut ability stats bundle.
     * @return Reference to Juggernaut ability stats.
     */
    static const JuggernautAbilities& juggernaut() { return m_juggernaut; }

    /**
     * @brief Returns the loaded Soldier ability stats bundle.
     * @return Reference to Soldier ability stats.
     */
    static const SoldierAbilities& soldier() { return m_soldier; }

    /**
     * @brief Looks up ability stats by network ability type.
     * @param type Ability type identifier.
     * @return Stats for @p type, or a static empty stats object if not found.
     */
    static const AbilityStats& getStats(AbilityType type) {
        auto it = m_stats.find(type);
        if (it != m_stats.end()) return it->second;
        return emptyStats();
    }

    /**
     * @brief Retrieves a named numeric parameter for an ability type.
     * @param type Ability type identifier.
     * @param key Parameter name (standard field or custom JSON key).
     * @param defaultVal Value returned when the parameter is missing.
     * @return Parameter value or @p defaultVal if not found.
     */
    static float param(AbilityType type, const std::string& key, float defaultVal = 0.f) {
        return param(getStats(type), key, defaultVal);
    }

    /**
     * @brief Retrieves a named numeric parameter from an ability stats object.
     * @param stats Ability stats to query.
     * @param key Parameter name (standard field or custom JSON key).
     * @param defaultVal Value returned when the parameter is missing.
     * @return Parameter value or @p defaultVal if not found.
     */
    static float param(const AbilityStats& stats, const std::string& key, float defaultVal = 0.f) {
        return paramFromStats(stats, key, defaultVal);
    }

private:
    // ==========================================
    // Per-Class JSON Loaders
    // ==========================================
    /**
     * @brief Parses Vanguard ability entries from a JSON class object.
     * @param classVal JSON object containing Vanguard ability definitions.
     * @param out Output structure populated with parsed stats.
     * @return Number of ability entries successfully loaded.
     */
    static int loadClass(const nlohmann::json& classVal, VanguardAbilities& out) {
        int count = 0;
        for (auto& [key, val] : classVal.items()) {
            if (!val.is_object()) continue;
            AbilityStats stats = parseEntry(val);
            ++count;
            if (key == "Decoy") {
                out.Decoy = stats;
                m_stats[AbilityType::VanguardDecoy] = stats;
            } else if (key == "Dash") {
                out.Dash = stats;
                m_stats[AbilityType::VanguardDash] = stats;
            } else if (key == "KatanaSlash") {
                out.KatanaSlash = stats;
                m_stats[AbilityType::VanguardKatanaSlash] = stats;
            } else if (key == "Ult") {
                out.Ult = stats;
            } else if (key == "ShurikenBurst") {
                out.ShurikenBurst = stats;
            } else {
                std::cerr << "[REGISTRY WARNING] Unknown Vanguard ability: " << key << "\n";
            }
        }
        return count;
    }

    /**
     * @brief Parses Medic ability entries from a JSON class object.
     * @param classVal JSON object containing Medic ability definitions.
     * @param out Output structure populated with parsed stats.
     * @return Number of ability entries successfully loaded.
     */
    static int loadClass(const nlohmann::json& classVal, MedicAbilities& out) {
        int count = 0;
        for (auto& [key, val] : classVal.items()) {
            if (!val.is_object()) continue;
            AbilityStats stats = parseEntry(val);
            ++count;
            if (key == "Teleport") {
                out.Teleport = stats;
                m_stats[AbilityType::MedicTeleport] = stats;
            } else if (key == "Orb") {
                out.Orb = stats;
                m_stats[AbilityType::MedicOrb] = stats;
            } else if (key == "Needle") {
                out.Needle = stats;
            } else if (key == "Barrier") {
                out.Barrier = stats;
                m_stats[AbilityType::MedicBarrier] = stats;
            } else if (key == "Passive") {
                out.Passive = stats;
            } else if (key == "Drone") {
                out.Drone = stats;
            } else {
                std::cerr << "[REGISTRY WARNING] Unknown Medic ability: " << key << "\n";
            }
        }
        return count;
    }

    /**
     * @brief Parses Juggernaut ability entries from a JSON class object.
     * @param classVal JSON object containing Juggernaut ability definitions.
     * @param out Output structure populated with parsed stats.
     * @return Number of ability entries successfully loaded.
     */
    static int loadClass(const nlohmann::json& classVal, JuggernautAbilities& out) {
        int count = 0;
        for (auto& [key, val] : classVal.items()) {
            if (!val.is_object()) continue;
            AbilityStats stats = parseEntry(val);
            ++count;
            if (key == "Dash") {
                out.Dash = stats;
                m_stats[AbilityType::JuggernautDash] = stats;
            } else if (key == "Repulsor") {
                out.Repulsor = stats;
                m_stats[AbilityType::JuggernautRepulsor] = stats;
            } else if (key == "BlackHole") {
                out.BlackHole = stats;
                m_stats[AbilityType::JuggernautBlackHole] = stats;
            } else if (key == "Ult") {
                out.Ult = stats;
            } else {
                std::cerr << "[REGISTRY WARNING] Unknown Juggernaut ability: " << key << "\n";
            }
        }
        return count;
    }

    /**
     * @brief Parses Soldier ability entries from a JSON class object.
     * @param classVal JSON object containing Soldier ability definitions.
     * @param out Output structure populated with parsed stats.
     * @return Number of ability entries successfully loaded.
     */
    static int loadClass(const nlohmann::json& classVal, SoldierAbilities& out) {
        int count = 0;
        for (auto& [key, val] : classVal.items()) {
            if (!val.is_object()) continue;
            AbilityStats stats = parseEntry(val);
            ++count;
            if (key == "HealField") {
                out.HealField = stats;
                m_stats[AbilityType::SoldierHealField] = stats;
            } else if (key == "Ult") {
                out.Ult = stats;
            } else if (key == "Sprint") {
                out.Sprint = stats;
            } else if (key == "Rocket") {
                out.Rocket = stats;
            } else if (key == "AutoAim") {
                out.AutoAim = stats;
            } else {
                std::cerr << "[REGISTRY WARNING] Unknown Soldier ability: " << key << "\n";
            }
        }
        return count;
    }

    // ==========================================
    // Parsing Helpers
    // ==========================================
    /**
     * @brief Converts a single JSON ability entry into an AbilityStats object.
     * @param val JSON object containing ability field values.
     * @return Parsed ability stats with known and custom parameters.
     */
    static AbilityStats parseEntry(const nlohmann::json& val) {
        AbilityStats stats;
        stats.cooldown = val.value("cooldown", 0.f);
        stats.damage = val.value("damage", 0.f);
        stats.knockback = val.value("knockback", 0.f);
        stats.radius = val.value("radius", 0.f);
        stats.range = val.value("range", 0.f);
        stats.duration = val.value("duration", 0.f);
        stats.speed = val.value("speed", 0.f);
        stats.lifetime = val.value("lifetime", 0.f);
        stats.heal = val.value("heal", 0.f);
        static const std::unordered_set<std::string> knownFields = {
            "cooldown", "damage", "knockback", "radius", "range",
            "duration", "speed", "lifetime", "heal"
        };
        for (auto& [key, value] : val.items()) {
            if (knownFields.count(key) || !value.is_number()) continue;
            stats.customParams[key] = value.get<float>();
        }
        return stats;
    }

    /**
     * @brief Resolves a parameter value from standard fields or custom parameters.
     * @param stats Ability stats to query.
     * @param key Parameter name to look up.
     * @param defaultVal Value returned when the parameter is missing.
     * @return Parameter value or @p defaultVal if not found.
     */
    static float paramFromStats(const AbilityStats& stats, const std::string& key, float defaultVal) {
        auto it = stats.customParams.find(key);
        if (it != stats.customParams.end()) return it->second;
        if (key == "cooldown") return stats.cooldown;
        if (key == "damage") return stats.damage;
        if (key == "knockback") return stats.knockback;
        if (key == "radius") return stats.radius;
        if (key == "range") return stats.range;
        if (key == "duration") return stats.duration;
        if (key == "speed") return stats.speed;
        if (key == "lifetime") return stats.lifetime;
        if (key == "heal") return stats.heal;
        return defaultVal;
    }

    /**
     * @brief Returns a shared empty stats object used as a fallback for unknown abilities.
     * @return Reference to a static zero-initialized AbilityStats instance.
     */
    static const AbilityStats& emptyStats() {
        static const AbilityStats empty{};
        return empty;
    }

    
    // Static Storage
    static inline std::unordered_map<AbilityType, AbilityStats> m_stats;
    static inline VanguardAbilities m_vanguard;
    static inline MedicAbilities m_medic;
    static inline JuggernautAbilities m_juggernaut;
    static inline SoldierAbilities m_soldier;
};
