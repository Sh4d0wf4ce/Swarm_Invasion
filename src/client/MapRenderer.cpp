#include "MapRenderer.hpp"

MapRenderer::MapRenderer(std::shared_ptr<MapGenerator> mapGen, float tileSize): m_mapGen(mapGen), m_tileSize(tileSize){
    m_wallShape.setSize({m_tileSize, m_tileSize});
    m_wallShape.setFillColor(sf::Color(80, 80, 80));

    m_floorShape.setSize({m_tileSize, m_tileSize});
    m_floorShape.setFillColor(sf::Color(150, 150, 150));
}

void MapRenderer::render(sf::RenderTarget& target){
    if(!m_mapGen) return;

    for(int x = 0; x < m_mapGen->getWidth(); x++){
        for(int y = 0; y < m_mapGen->getHeight(); y++){
            sf::Vector2f pos(x * m_tileSize, y * m_tileSize);

            if(m_mapGen->getTile(x, y) == TileType::Wall){
                m_wallShape.setPosition(pos);
                target.draw(m_wallShape);
            }else{
                m_floorShape.setPosition(pos);
                target.draw(m_floorShape);
            }
        }
    }
}
