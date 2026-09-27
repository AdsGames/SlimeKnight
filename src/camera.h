#pragma once

#include <asw/asw.h>

#include "globals.h"

// Smooth follow camera with screen shake
class Camera {
 public:
  Camera() = default;

  // Set the world size the camera is clamped to
  void set_bounds(const asw::Quadf& bounds);

  // Point on screen the target is kept at
  void set_anchor(const asw::Vec2f& anchor);

  // Jump straight to a target
  void snap(const asw::Vec2f& target);

  // Ease towards a target
  void follow(const asw::Vec2f& target, float dt);

  // Add screen shake, strength in pixels
  void shake(float strength);

  // Top left of the view in world space, includes shake
  asw::Vec2f get_offset() const;

  // Convert a world position to screen space
  asw::Vec2f to_screen(const asw::Vec2f& world) const;

  // Convert a world quad to screen space
  asw::Quadf to_screen(const asw::Quadf& world) const;

  // Visible world area
  asw::Quadf get_view() const;

 private:
  void clamp();

  asw::Vec2f position;
  asw::Vec2f anchor{SCREEN_W / 2.0F, SCREEN_H / 2.0F};
  asw::Vec2f shake_offset;
  asw::Quadf bounds;
  float shake_strength{0.0F};
};
