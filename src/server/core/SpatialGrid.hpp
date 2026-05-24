#pragma once

#include <vector>
#include <cstdint>
#include <SFML/System/Vector2.hpp>

class SpatialGrid{
public:
    SpatialGrid(float mapWidth, float mapHeight, float cellSize) : m_cellSize(cellSize){
        m_cols = static_cast<int>(std::ceil(mapWidth / cellSize));
        m_rows = static_cast<int>(std::ceil(mapHeight / cellSize));
        m_cells.resize(m_cols * m_rows);
    }

    void clear(){
        for(auto& cell : m_cells){
            cell.clear();
        }
    }

    void insert(sf::Vector2f position, std::uint32_t entityId){
        int x = static_cast<int>(position.x / m_cellSize);
        int y = static_cast<int>(position.y / m_cellSize);

        if(x >= 0 && x < m_cols && y >= 0 && y < m_rows){
            m_cells[y * m_cols + x].push_back(entityId);
        }
    }

    std::vector<std::uint32_t> getNearby(sf::Vector2f position) const {
        std::vector<std::uint32_t> result;
        result.reserve(16);

        int cx = static_cast<int>(position.x / m_cellSize);
        int cy = static_cast<int>(position.y / m_cellSize);

        for (int y = cy - 1; y <= cy + 1; ++y) {
            for (int x = cx - 1; x <= cx + 1; ++x) {
                if (x >= 0 && x < m_cols && y >= 0 && y < m_rows) {
                    const auto& cell = m_cells[y * m_cols + x];
                    result.insert(result.end(), cell.begin(), cell.end());
                }
            }
        }
        return result;
    }


private:
    float m_cellSize;
    int m_cols;
    int m_rows;
    std::vector<std::vector<std::uint32_t>> m_cells;
};