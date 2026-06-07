#include "MapGenerator.hpp"

/**
 * @brief Constructs a map generator and initializes an empty floor grid.
 * @param width Map width in tiles.
 * @param height Map height in tiles.
 */
MapGenerator::MapGenerator(int width, int height): m_width(width), m_height(height), m_fillPercent(Config::MAP_FILL_PERCENT){
    m_map.resize(m_width, std::vector<TileType>(m_height, TileType::Floor));
}

/**
 * @brief Generates a cave-like map from random wall fill, smoothing passes, and a cleared spawn area.
 * @param seed Random seed for reproducible map generation.
 */
void MapGenerator::generate(int seed){
    // --- Random initial wall fill with border walls ---
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 100);
    for(int x = 0; x < m_width; x++){
        for(int y = 0; y < m_height; y++){
            if(x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1){
                m_map[x][y] = TileType::Wall;
            }else{
                m_map[x][y] = (dist(rng) < m_fillPercent) ? TileType::Wall : TileType::Floor;
            }
        }
    }

    // --- Smooth cave-like topology ---
    for(int i = 0; i < 5; i++){
        smoothMap();
    }

     // --- Clear spawn-safe area at map center ---
     int centerX = m_width / 2;
     int centerY = m_height / 2;
     float clearRadius = 8.0f;
     for(int x = centerX - clearRadius; x <= centerX + clearRadius; x++){
        for(int y = centerY - clearRadius; y <= centerY + clearRadius; y++){
             if(x > 0 && x < m_width - 1 && y > 0 && y < m_height -1){
                float distSq = std::pow(x - centerX, 2) + std::pow(y - centerY, 2);
                if(distSq <= clearRadius * clearRadius){
                    m_map[x][y] = TileType::Floor;
                }
             }
        }
     }
}

/**
 * @brief Applies one cellular-automata smoothing pass to the tile grid.
 */
void MapGenerator::smoothMap(){
    std::vector<std::vector<TileType>> newMap = m_map;
    for(int x = 0; x < m_width; x++){
        for(int y = 0; y < m_height; y++){
            int surroundingWallsCount = getSurroundingWallCount(x, y);
            if(surroundingWallsCount > 4){
                newMap[x][y] = TileType::Wall;
            }else if(surroundingWallsCount < 4){
                newMap[x][y] = TileType::Floor;
            }
        }
    }
    m_map = newMap;
}

/**
 * @brief Counts wall tiles in the 3x3 neighborhood around a grid cell.
 * @param gridX Tile column index.
 * @param gridY Tile row index.
 * @return Number of neighboring wall tiles, treating out-of-bounds as walls.
 */
int MapGenerator::getSurroundingWallCount(int gridX, int gridY) const{
    int wallCount = 0;
    for(int neighbourX = gridX - 1; neighbourX <= gridX + 1; neighbourX++){
        for(int neighbourY = gridY - 1; neighbourY <= gridY + 1; neighbourY++){
            if(neighbourX < 0 || neighbourX >= m_width || neighbourY < 0 || neighbourY >= m_height){
                wallCount++;
            }else{
                if(neighbourX == gridX  && neighbourY == gridY) continue;
                if(m_map[neighbourX][neighbourY] == TileType::Wall) wallCount++;
            }
        }
    }
    return wallCount;
}

/**
 * @brief Returns the tile type at grid coordinates.
 * @param x Tile column index.
 * @param y Tile row index.
 * @return Tile at (@p x, @p y), or TileType::Wall for out-of-bounds coordinates.
 */
TileType MapGenerator::getTile(int x, int y) const{
    if(x < 0 || x >= m_width || y < 0 || y >= m_height) return TileType::Wall;
    return m_map[x][y];
}

/**
 * @brief Tests whether a circular entity overlaps any wall tile using corner sample points.
 * @param pos World-space center position of the entity.
 * @param radius Collision radius of the entity in world units.
 * @return True if any sample point intersects a wall tile.
 */
bool MapGenerator::checkCollision(const sf::Vector2f& pos, float radius) const {
    // --- Test corner sample points against wall tiles ---
    float hitBoxOffset = radius * 0.8f;
    sf::Vector2f points[4] = {
        {pos.x - hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x - hitBoxOffset, pos.y + hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y + hitBoxOffset}
    };
    for(const auto& p: points){
        int gridX = static_cast<int>(p.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(p.y / Config::TILE_SIZE);
        if(getTile(gridX, gridY) == TileType::Wall) return true;
    }
    return false;
}
