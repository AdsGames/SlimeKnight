#pragma once

#include <asw/asw.h>
#include <deque>


// Input for one step, read from keyboard and controller
struct KnightInput {
  asw::Vec2f move;
  bool slash{false};
  bool dash{false};
  bool hammer_held{false};
  bool hammer{false};
  // Care package key, -1 for none
  int package{-1};

  static KnightInput read();
};

// The player
class Knight {
 public:
  void init();
  void reset(const asw::Vec2f& position);

  void update(float dt, const KnightInput& input);
  void draw(const asw::Camera& camera) const;

  // Solid body around the feet
  asw::Quadf body() const;

  // Middle of the sprite
  asw::Vec2f center() const;

  // Try to start a sword swing, returns true on success
  bool try_swing();

  // Try to start a dash, returns true on success
  bool try_dash(const asw::Vec2f& direction);

  bool hammer_ready() const;

  // Spend the hammer cost
  void use_hammer();

  // Apply damage, returns false when the knight is invulnerable
  bool hurt(int amount, const asw::Vec2f& from);

  bool is_swinging() const { return swing_timer > 0.0F; }
  bool is_dashing() const { return dash_timer > 0.0F; }
  bool is_dead() const { return health <= 0.0F; }

  // Area hit by the current swing
  asw::Vec2f swing_center() const;

  // Stats
  asw::Vec2f position;
  asw::Vec2f velocity;
  asw::Vec2f facing{1.0F, 0.0F};
  float health{100.0F};
  float energy{100.0F};
  float power{0.0F};
  int regen_level{0};
  int speed_level{0};

  // Energy spent this level
  float energy_used{0.0F};

  static constexpr float MAX_STAT = 100.0F;
  static constexpr float SWING_COST = 8.0F;
  static constexpr float DASH_COST = 20.0F;
  static constexpr float HAMMER_COST = 30.0F;
  static constexpr float SWING_RADIUS = 80.0F;
  static constexpr int MAX_REGEN_LEVEL = 2;

 private:
  void spend_energy(float amount);

  bool face_right{true};
  bool hammer_up{false};
  float swing_timer{0.0F};
  float swing_cooldown{0.0F};
  float dash_timer{0.0F};
  float dash_cooldown{0.0F};
  float invulnerable_timer{0.0F};
  float step_timer{0.0F};
  float trail_timer{0.0F};

  std::deque<asw::Vec2f> trail;

  asw::Texture tex_left;
  asw::Texture tex_right;
  asw::Texture tex_left_slash;
  asw::Texture tex_right_slash;
  asw::Texture tex_left_hammer;
  asw::Texture tex_right_hammer;
};
