#include "camera.h"

#include <algorithm>
#include <cmath>

void Camera::set_bounds(const asw::Quadf& bounds) {
  this->bounds = bounds;
}

void Camera::set_anchor(const asw::Vec2f& anchor) {
  this->anchor = anchor;
}

void Camera::snap(const asw::Vec2f& target) {
  position = target - anchor;
  shake_strength = 0.0F;
  shake_offset = asw::Vec2f(0, 0);
  clamp();
}

void Camera::follow(const asw::Vec2f& target, float dt) {
  // Frame rate independent ease
  const float t = 1.0F - std::exp(-8.0F * dt);
  position = position + ((target - anchor) - position) * t;
  clamp();

  // Shake decays quickly
  shake_strength = std::max(0.0F, shake_strength - (60.0F * dt));
  if (shake_strength > 0.0F) {
    shake_offset = asw::Vec2f(asw::random::between(-1.0F, 1.0F),
                              asw::random::between(-1.0F, 1.0F)) *
                   shake_strength;
  } else {
    shake_offset = asw::Vec2f(0, 0);
  }
}

void Camera::shake(float strength) {
  shake_strength = std::min(30.0F, std::max(shake_strength, strength));
}

asw::Vec2f Camera::get_offset() const {
  return position + shake_offset;
}

asw::Vec2f Camera::to_screen(const asw::Vec2f& world) const {
  return world - get_offset();
}

asw::Quadf Camera::to_screen(const asw::Quadf& world) const {
  return {world.position - get_offset(), world.size};
}

asw::Quadf Camera::get_view() const {
  return {get_offset(), asw::Vec2f(SCREEN_W, SCREEN_H)};
}

void Camera::clamp() {
  const float max_x = bounds.position.x + bounds.size.x - SCREEN_W;
  const float max_y = bounds.position.y + bounds.size.y - SCREEN_H;
  position.x = std::clamp(position.x, bounds.position.x, max_x);
  position.y = std::clamp(position.y, bounds.position.y, max_y);
}
