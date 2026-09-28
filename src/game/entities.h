#pragma once

#include <asw/asw.h>

// Kinds of slime
enum class SlimeType {
  Green,
  Red,
  Purple,
  Mini,
  Boss,
};

// Care package types, index matches the number key used to call them in
enum class PackageType {
  Regen,
  Health,
  Energy,
  Speed,
};

// Hopping slime that chases the knight
struct Slime {
  SlimeType type{SlimeType::Green};
  asw::Vec2f position;
  asw::Vec2f velocity;
  float radius{26.0F};
  int health{1};
  int max_health{1};
  float hop_timer{0.0F};
  float hop_period{1.0F};
  float hop_speed{300.0F};
  // Four frame wobble, 0.2 seconds a frame
  asw::Animation animation{4, 0.2F};
  float hurt_timer{0.0F};
  float spawn_timer{0.0F};
  // Boss charge state
  float charge_timer{0.0F};
  float telegraph_timer{0.0F};
  bool alive{true};

  int damage() const;

  // Size the slime is drawn at
  asw::Vec2f draw_size() const;

  // Body used for hits and contact, the solid middle of the sprite
  asw::Quadf hitbox() const;
};

// Slime tower, destroyed with the hammer slam
struct Tower {
  asw::Vec2f base;
  bool destroyed{false};

  // Solid footprint at the base of the tower
  asw::Quadf footprint() const;

  // Full sprite area
  asw::Quadf sprite_area() const;

  static constexpr float WIDTH = 176.0F;
  static constexpr float HEIGHT = 246.0F;
};

// Solid block of scenery
struct Ruin {
  asw::Quadf bounds;

  static constexpr float SIZE = 72.0F;
};

// Care package that drops in on a parachute
struct Package {
  PackageType type{PackageType::Health};
  asw::Vec2f target;
  float fall_timer{0.0F};
  bool landed{false};
  bool collected{false};

  // Wind while it falls, stopped when it lands
  asw::sound::SoundHandle wind;

  static constexpr float FALL_TIME = 1.4F;
  static constexpr float BOX_SIZE = 40.0F;

  asw::Quadf box() const;
};
