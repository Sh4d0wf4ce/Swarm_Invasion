#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

/**
 * @brief Utility functions for angle normalization and arc-based collision geometry.
 *
 * Used by abilities and barriers that test whether entities lie within a curved wall segment.
 */
namespace SectorMath {

/**
 * @brief Wraps a radian angle into the range [-pi, pi].
 * @param radians Input angle in radians.
 * @return Normalized angle in radians.
 */
inline float normalizeAngle(float radians) {
    constexpr float twoPi = 6.2831853f;
    while (radians > 3.14159265f) radians -= twoPi;
    while (radians < -3.14159265f) radians += twoPi;
    return radians;
}


// Arc Geometry
/**
 * @brief Computes the center of an arc circle given an origin, facing direction, and standoff distance.
 * @param origin World-space origin point of the arc owner.
 * @param facingAngle Direction the arc faces, in radians.
 * @param arcRadius Outer radius of the arc.
 * @param standoff Distance from the arc edge back toward the origin.
 * @return World-space center of the arc circle.
 */
inline sf::Vector2f arcCircleCenter(
    sf::Vector2f origin,
    float facingAngle,
    float arcRadius,
    float standoff) {
    const float offset = arcRadius - standoff;
    return origin - sf::Vector2f(std::cos(facingAngle), std::sin(facingAngle)) * offset;
}

/**
 * @brief Tests whether a circular point intersects a thick arc wall segment.
 * @param center World-space center of the arc circle.
 * @param facingAngle Direction the arc faces, in radians.
 * @param arcRadius Outer radius of the arc.
 * @param halfSpan Half angular span of the arc, in radians.
 * @param wallThickness Thickness of the arc wall.
 * @param point World-space center of the tested point.
 * @param pointRadius Collision radius of the tested point.
 * @return True if @p point overlaps the arc wall; false otherwise.
 */
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
