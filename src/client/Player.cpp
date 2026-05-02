#include "Player.hpp"

Player::Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass): Entity(id, startPos), m_isFocused(true), m_class(pClass){
    const auto& stats = HeroRegistry::getStats(pClass);

    m_maxHp = stats.maxHp;
    m_hp = stats.maxHp;
    m_speed = stats.speed;
    
    m_shape.setRadius(stats.radius);
    m_shape.setFillColor(stats.color);
    m_shape.setOrigin({stats.radius, stats.radius});
    m_shape.setPosition(m_position);
}

void Player::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    sf::Vector2f movement(0.0f, 0.0f);

    if(m_isFocused){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.0f;
    }

    float len = movement.length();
    if(len > 0.0f) movement /= len;

    sf::Vector2f velocity = movement * m_speed * deltaTime.asSeconds();

    sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
    if(!checkCollision(nextPosX, map)){
        m_position.x = nextPosX.x;
    }

    sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
    if(!checkCollision(nextPosY, map)){
        m_position.y = nextPosY.y;
    }

    m_shape.setPosition(m_position);
}

void Player::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
    drawHealthBar(target, 30.0f);
}

void Player::setFocused(bool focuesd){
    m_isFocused = focuesd;
}

bool Player::checkCollision(const sf::Vector2f& pos, const std::shared_ptr<MapGenerator>& map){
    if(!map) return false;

    float hitBoxOffset = HeroRegistry::getStats(m_class).radius * 0.8f;

    sf::Vector2f points[4] = {
        {pos.x - hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x - hitBoxOffset, pos.y + hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y + hitBoxOffset}
    };

    for(const auto& p: points){
        int gridX = static_cast<int>(p.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(p.y / Config::TILE_SIZE);
    
        if(map->getTile(gridX, gridY) == TileType::Wall)
            return true;
    }

    return false;
}
