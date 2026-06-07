#pragma once
#include "NetworkProtocol.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <optional>
#include <random>
#include <algorithm>


/**
 * @brief Hero stat fields that level-up upgrades can modify.
 */
enum class UpgradeStat {
    MaxHP,
    Speed,
    Damage,
    Cooldown
};

/**
 * @brief Metadata and effect definition for a single level-up upgrade or augment card.
 */
struct UpgradeDefinition {
    std::string id;
    std::string name;
    std::string description;
    float weight{1.0f};
    bool isAugment{false};
    std::optional<PlayerClass> targetClass;
    UpgradeStat stat{UpgradeStat::MaxHP};
    float modifierValue{0.0f};
};

/**
 * @brief Central registry that loads upgrade definitions and selects weighted level-up offers.
 */
class UpgradeRegistry {
public:
    // ==========================================
    // Config Loading & Lookup
    // ==========================================
    /**
     * @brief Loads upgrade definitions from a JSON file and populates the registry.
     * @param filepath Path to the upgrades JSON configuration file.
     */
    static void loadConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[REGISTRY ERROR] Could not open " << filepath << "!\n";
            return;
        }

        nlohmann::json j;
        file >> j;
        file.close();
        m_upgrades.clear();
        std::unordered_map<std::string, PlayerClass> classMap = {
            {"Soldier", PlayerClass::Soldier},
            {"Medic", PlayerClass::Medic},
            {"Juggernaut", PlayerClass::Juggernaut},
            {"Vanguard", PlayerClass::Vanguard}
        };
        std::unordered_map<std::string, UpgradeStat> statMap = {
            {"MaxHP", UpgradeStat::MaxHP},
            {"Speed", UpgradeStat::Speed},
            {"Damage", UpgradeStat::Damage},
            {"Cooldown", UpgradeStat::Cooldown}
        };
        for (auto& [key, val] : j.items()) {
            UpgradeDefinition def;
            def.id = key;
            def.name = val["name"];
            def.description = val["description"];
            def.weight = val["weight"];
            def.isAugment = val.value("isAugment", false);
            def.modifierValue = val["modifierValue"];
            if (val.contains("targetClass") && !val["targetClass"].is_null()) {
                std::string classStr = val["targetClass"];
                if (classMap.count(classStr)) {
                    def.targetClass = classMap[classStr];
                }
            }

            std::string statStr = val["statToModify"];
            if (statMap.count(statStr)) {
                def.stat = statMap[statStr];
            }
            m_upgrades[key] = def;
        }

        std::cout << "[REGISTRY] Successfully loaded " << filepath << " (" << m_upgrades.size() << " upgrades)\n";
    }

    /**
     * @brief Looks up an upgrade definition by its unique identifier.
     * @param id Upgrade identifier string.
     * @return Pointer to the definition, or nullptr if not found.
     */
    static const UpgradeDefinition* getById(const std::string& id) {
        auto it = m_upgrades.find(id);
        if (it == m_upgrades.end()) return nullptr;
        return &it->second;
    }

    /**
     * @brief Returns the full map of loaded upgrade definitions.
     * @return Reference to the internal upgrade definition map.
     */
    static const std::unordered_map<std::string, UpgradeDefinition>& getAll() {
        return m_upgrades;
    }

    // ==========================================
    // Offer Selection
    // ==========================================
    /**
     * @brief Builds a candidate pool of upgrades eligible for a level-up offer.
     * @param playerClass Hero class receiving the offer.
     * @param wantAugment True to include augments; false for standard stat upgrades.
     * @return Vector of pointers to eligible upgrade definitions.
     */
    static std::vector<const UpgradeDefinition*> buildPool(
        PlayerClass playerClass,
        bool wantAugment) {
        std::vector<const UpgradeDefinition*> pool;
        for (const auto& [id, def] : m_upgrades) {
            (void)id;
            if (def.isAugment != wantAugment) continue;
            if (def.targetClass.has_value() && def.targetClass.value() != playerClass) continue;
            if (!def.isAugment && def.targetClass.has_value() && def.targetClass.value() != playerClass) continue;
            pool.push_back(&def);
        }
        return pool;
    }

    /**
     * @brief Selects a weighted random subset of upgrades from a candidate pool without replacement.
     * @param pool Candidate upgrade definitions to draw from.
     * @param count Maximum number of upgrades to pick.
     * @return Vector of chosen upgrade identifier strings.
     */
    static std::vector<std::string> pickWeightedRandom(
        const std::vector<const UpgradeDefinition*>& pool,
        int count) {
        std::vector<std::string> result;
        if (pool.empty() || count <= 0) return result;
        std::vector<const UpgradeDefinition*> remaining = pool;
        int picks = std::min(count, static_cast<int>(remaining.size()));
        for (int i = 0; i < picks; ++i) {
            float totalWeight = 0.0f;
            for (const auto* def : remaining) {
                totalWeight += def->weight;
            }
            if (totalWeight <= 0.0f) break;
            float roll = (static_cast<float>(std::rand()) / RAND_MAX) * totalWeight;
            float cumulative = 0.0f;
            auto chosenIt = remaining.end();
            for (auto it = remaining.begin(); it != remaining.end(); ++it) {
                cumulative += (*it)->weight;
                if (roll <= cumulative) {
                    chosenIt = it;
                    break;
                }
            }
            if (chosenIt == remaining.end()) {
                chosenIt = remaining.begin();
            }
            result.push_back((*chosenIt)->id);
            remaining.erase(chosenIt);
        }
        return result;
    }

private:
    static inline std::unordered_map<std::string, UpgradeDefinition> m_upgrades;
};
