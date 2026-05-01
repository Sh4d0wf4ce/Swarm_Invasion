#include "Player.hpp"

Player::Player(std::uint32_t id, const sf::Vector2f& startPos): Entity(id, startPos), m_speed(300.0f), m_isFocused(true){
    m_shape.setRadius(20.0f);
    m_shape.setFillColor(sf::Color::Red);
    m_shape.setOrigin({20.0f, 20.0f});
    m_shape.setPosition(m_position);
}

void Player::update(sf::Time deltaTime){
    sf::Vector2f movement(0.0f, 0.0f);

    if(m_isFocused){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.0f;
    }

    m_position += movement * m_speed * deltaTime.asSeconds();
}

void Player::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}

void Player::setFocused(bool focuesd){
    m_isFocused = focuesd;
}
