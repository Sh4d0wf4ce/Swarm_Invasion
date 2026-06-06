#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace SectorMath {

inline float normalizeAngle(float radians) {
    constexpr float twoPi = 6.2831853f;
    while (radians > 3.14159265f) radians -= twoPi;
    while (radians < -3.14159265f) radians += twoPi;
    return radians;
}

inline sf::Vector2f arcCircleCenter(
    sf::Vector2f origin,
    float facingAngle,
    float arcRadius,
    float standoff) {
    const float offset = arcRadius - standoff;
    return origin - sf::Vector2f(std::cos(facingAngle), std::sin(facingAngle)) * offset;
}

inline bool isInArcWall(
    sf::Vector2f center,
    float facingAngle,
    float arcRadius,
    float halfSpan,
    float wallThickness,
    sf::Vector2f point,
    float pointRadius) {
    sf::Vector2f delta = point - center;
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    float innerBound = arcRadius - wallThickness - pointRadius;
    float outerBound = arcRadius + wallThickness + pointRadius;
    if (dist < innerBound || dist > outerBound) return false;

    float pointAngle = std::atan2(delta.y, delta.x);
    float diff = normalizeAngle(pointAngle - facingAngle);
    return std::abs(diff) <= halfSpan;
}

}
