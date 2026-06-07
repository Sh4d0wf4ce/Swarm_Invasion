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
struct VanguardAbilities {
    AbilityStats Decoy;
    AbilityStats Dash;
    AbilityStats KatanaSlash;
    AbilityStats Ult;
    AbilityStats ShurikenBurst;
};
struct MedicAbilities {
    AbilityStats Teleport;
    AbilityStats Orb;
    AbilityStats Needle;
    AbilityStats Barrier;
    AbilityStats Passive;
    AbilityStats Drone;
};
struct JuggernautAbilities {
    AbilityStats Dash;
    AbilityStats Repulsor;
    AbilityStats BlackHole;
    AbilityStats Ult;
};
struct SoldierAbilities {
    AbilityStats HealField;
    AbilityStats Ult;
    AbilityStats Sprint;
    AbilityStats Rocket;
    AbilityStats AutoAim;
};

class AbilityRegistry {
public:
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
    static const VanguardAbilities& vanguard() { return m_vanguard; }
    static const MedicAbilities& medic() { return m_medic; }
    static const JuggernautAbilities& juggernaut() { return m_juggernaut; }
    static const SoldierAbilities& soldier() { return m_soldier; }
    static const AbilityStats& getStats(AbilityType type) {
        auto it = m_stats.find(type);
        if (it != m_stats.end()) return it->second;
        return emptyStats();
    }

    static float param(AbilityType type, const std::string& key, float defaultVal = 0.f) {
        return param(getStats(type), key, defaultVal);
    }

    static float param(const AbilityStats& stats, const std::string& key, float defaultVal = 0.f) {
        return paramFromStats(stats, key, defaultVal);
    }

private:
    // ==========================================
    // Per-Class JSON Loaders
    // ==========================================
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
