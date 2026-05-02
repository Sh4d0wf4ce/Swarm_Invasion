#include "MapGenerator.hpp"

MapGenerator::MapGenerator(int width, int height): m_width(width), m_height(height), m_fillPercent(Config::MAP_FILL_PERCENT){
    m_map.resize(m_width, std::vector<TileType>(m_height, TileType::Floor));
}

void MapGenerator::generate(int seed){
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

    for(int i = 0; i < 5; i++){
        smoothMap();
    }
}

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

TileType MapGenerator::getTile(int x, int y) const{
    if(x < 0 || x >= m_width || y < 0 || y >= m_height) return TileType::Wall;
    return m_map[x][y];
}