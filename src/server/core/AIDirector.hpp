#pragma once

#include "ServerTypes.hpp"
#include "../enemies/ServerEnemy.hpp"
#include "MapGenerator.hpp"

#include <map>
#include <memory>
#include <random>
#include <vector>
#include <queue>

class AIDirector{
public:
    AIDirector();

    void updateWaves(sf::Time deltaTime,
                     std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& enemies,
                     const std::map<std::uint32_t, ClientInfo>& clients,
                     std::shared_ptr<MapGenerator> map,
                     std::uint32_t& entityCounter);

    std::vector<std::uint32_t> updateBehaviours(sf::Time deltaTime,
                                                std::map<std::uint32_t, std::unique_ptr<ServerEnemy>>& enemies,
                                                std::map<std::uint32_t, ClientInfo>& clients,
                                                std::shared_ptr<MapGenerator> map,
											    std::vector<EnemyShootEvent>& outShootEvents);

private:
	void buildFlowField(std::shared_ptr<MapGenerator> map, const std::map<uint32_t, ClientInfo>& clients);

	sf::Clock m_pathFindingTimer;
	std::vector <std::vector<int>> m_flowField;

	std::mt19937 m_rng;
	int m_currentWave;
	sf::Clock m_waveTimer;
	sf::Clock m_spawnTimer;
	float m_currentSpawnRate;
};