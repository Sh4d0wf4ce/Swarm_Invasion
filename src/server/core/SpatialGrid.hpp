#pragma once
#include <vector>
#include <cstdint>
#include <SFML/System/Vector2.hpp>

/**
 * @brief Uniform grid spatial hash for broad-phase entity neighbor queries.
 *
 * Divides the map into fixed-size cells and stores entity IDs per cell to avoid
 * scanning all entities when computing local interactions.
 */
class SpatialGrid{
public:
    /**
     * @brief Constructs a grid covering the given map dimensions.
     * @param mapWidth Map width in world units.
     * @param mapHeight Map height in world units.
     * @param cellSize Size of each grid cell in world units.
     */
    SpatialGrid(float mapWidth, float mapHeight, float cellSize) : m_cellSize(cellSize){
        m_cols = static_cast<int>(std::ceil(mapWidth / cellSize));
        m_rows = static_cast<int>(std::ceil(mapHeight / cellSize));
        m_cells.resize(m_cols * m_rows);
    }

    /**
     * @brief Removes all entity IDs from every cell without changing grid dimensions.
     */
    void clear(){
        for(auto& cell : m_cells){
            cell.clear();
        }
    }


    /**
     * @brief Inserts an entity ID into the cell containing the given position.
     * @param position World position of the entity.
     * @param entityId Unique entity identifier to store.
     */
    void insert(sf::Vector2f position, std::uint32_t entityId){
        int x = static_cast<int>(position.x / m_cellSize);
        int y = static_cast<int>(position.y / m_cellSize);
        if(x >= 0 && x < m_cols && y >= 0 && y < m_rows){
            m_cells[y * m_cols + x].push_back(entityId);
        }
    }

    /**
     * @brief Returns entity IDs in the 3x3 neighborhood around a position.
     * @param position World position to query around.
     * @return Combined list of entity IDs from adjacent cells.
     */
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
