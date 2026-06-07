#pragma once
#include "Config.hpp"
#include <SFML/System.hpp>
#include <vector>
#include <random>


// Map Tile Types
enum class TileType{
    Floor,
    Wall
};

class MapGenerator{
public:
    MapGenerator(int width, int height);
    void generate(int seed);


    TileType getTile(int x, int y) const;
    int getWidth() const {return m_width;}
    int getHeight() const {return m_height;}
    bool checkCollision(const sf::Vector2f& pos, float radius) const;

private:
    void smoothMap();
    int getSurroundingWallCount(int gridX, int gridY) const;

    int m_width;
    int m_height;
    int m_fillPercent;
    std::vector<std::vector<TileType>> m_map;
};
