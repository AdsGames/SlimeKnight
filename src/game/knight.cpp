#include "knight.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "audio.h"

namespace {
constexpr float SPRITE_W = 60.0F;
constexpr float SPRITE_H = 111.0F;
constexpr float BASE_SPEED = 290.0F;
constexpr float SPEED_PER_LEVEL = 45.0F;
constexpr float BASE_REGEN = 14.0F;
constexpr float REGEN_PER_LEVEL = 8.0F;
constexpr float SWING_TIME = 0.15F;
constexpr float SWING_COOLDOWN = 0.27F;
constexpr float DASH_TIME = 0.17F;
constexpr float DASH_SPEED = 1000.0F;
constexpr float DASH_COOLDOWN = 0.45F;
constexpr float INVULNERABLE_TIME = 0.8F;

using asw::input::ControllerAxis;
using asw::input::ControllerButton;
using asw::input::Key;
}  // namespace

KnightInput KnightInput::read() {
  KnightInput input;

  if (asw::input::get_key(Key::W) || asw::input::get_key(Key::Up)) {
    input.move.y -= 1.0F;
  }
  if (asw::input::get_key(Key::S) || asw::input::get_key(Key::Down)) {
    input.move.y += 1.0F;
  }
  if (asw::input::get_key(Key::A) || asw::input::get_key(Key::Left)) {
    input.move.x -= 1.0F;
  }
  if (asw::input::get_key(Key::D) || asw::input::get_key(Key::Right)) {
    input.move.x += 1.0F;
  }

  input.slash = asw::input::get_key(Key::Space) || asw::input::get_key(Key::J);
  input.dash = asw::input::get_key_down(Key::LShift) ||
               asw::input::get_key_down(Key::RShift) ||
               asw::input::get_key_down(Key::K);
  input.hammer_held =
      asw::input::get_key(Key::E) || asw::input::get_key(Key::L);
  input.hammer =
      asw::input::get_key_down(Key::E) || asw::input::get_key_down(Key::L);

  if (asw::input::get_key_down(Key::Num1)) {
    input.package = 0;
  } else if (asw::input::get_key_down(Key::Num2)) {
    input.package = 1;
  } else if (asw::input::get_key_down(Key::Num3)) {
    input.package = 2;
  } else if (asw::input::get_key_down(Key::Num4)) {
    input.package = 3;
  }

  // Controller
  if (asw::input::get_controller_count() > 0) {
    const asw::Vec2f stick(
        asw::input::get_controller_axis(0, ControllerAxis::LeftX),
        asw::input::get_controller_axis(0, ControllerAxis::LeftY));
    if (stick.magnitude() > 0.25F) {
      input.move = stick;
    }

    input.slash = input.slash ||
                  asw::input::get_controller_button(0, ControllerButton::A);
    input.dash = input.dash ||
                 asw::input::get_controller_button_down(0, ControllerButton::B);
    input.hammer_held = input.hammer_held || asw::input::get_controller_button(
                                                 0, ControllerButton::Y);
    input.hammer = input.hammer || asw::input::get_controller_button_down(
                                       0, ControllerButton::Y);

    if (asw::input::get_controller_button_down(0, ControllerButton::DPadUp)) {
      input.package = 0;
    } else if (asw::input::get_controller_button_down(
                   0, ControllerButton::DPadLeft)) {
      input.package = 1;
    } else if (asw::input::get_controller_button_down(
                   0, ControllerButton::DPadRight)) {
      input.package = 2;
    } else if (asw::input::get_controller_button_down(
                   0, ControllerButton::DPadDown)) {
      input.package = 3;
    }
  }

  // Diagonals are not faster
  if (input.move.magnitude() > 1.0F) {
    input.move = input.move / input.move.magnitude();
  }

  return input;
}

void Knight::init() {
  tex_left = asw::assets::load_texture("assets/images/game/knight_l.png");
  tex_right = asw::assets::load_texture("assets/images/game/knight_r.png");
  tex_left_slash =
      asw::assets::load_texture("assets/images/game/knight_l_slash.png");
  tex_right_slash =
      asw::assets::load_texture("assets/images/game/knight_r_slash.png");
  tex_left_hammer =
      asw::assets::load_texture("assets/images/game/knight_l_hammer.png");
  tex_right_hammer =
      asw::assets::load_texture("assets/images/game/knight_r_hammer.png");
}

void Knight::reset(const asw::Vec2f& position) {
  this->position = position;
  velocity = asw::Vec2f(0, 0);
  facing = asw::Vec2f(1, 0);
  face_right = true;
  health = MAX_STAT;
  energy = MAX_STAT;
  power = 0.0F;
  regen_level = 0;
  speed_level = 0;
  energy_used = 0.0F;
  swing_timer = 0.0F;
  swing_cooldown = 0.0F;
  dash_timer = 0.0F;
  dash_cooldown = 0.0F;
  invulnerable_timer = 0.0F;
  step_timer = 0.0F;
  trail.clear();
}

void Knight::update(float dt, const KnightInput& input) {
  swing_timer = std::max(0.0F, swing_timer - dt);
  swing_cooldown = std::max(0.0F, swing_cooldown - dt);
  dash_timer = std::max(0.0F, dash_timer - dt);
  dash_cooldown = std::max(0.0F, dash_cooldown - dt);
  invulnerable_timer = std::max(0.0F, invulnerable_timer - dt);

  hammer_up = input.hammer_held && hammer_ready();

  // Facing follows movement
  if (input.move.magnitude() > 0.1F && !is_dashing()) {
    facing = input.move / input.move.magnitude();
    if (std::abs(input.move.x) > 0.1F) {
      face_right = input.move.x > 0.0F;
    }
  }

  // Move
  if (is_dashing()) {
    position += facing * (DASH_SPEED * dt);
  } else {
    float speed =
        BASE_SPEED + (SPEED_PER_LEVEL * static_cast<float>(speed_level));
    if (hammer_up) {
      speed *= 0.75F;
    }
    position += input.move * (speed * dt);

    // Footsteps
    if (input.move.magnitude() > 0.1F) {
      step_timer -= dt;
      if (step_timer <= 0.0F) {
        Audio::play(Sfx::Step, 0.25F);
        step_timer = 0.38F;
      }
    }
  }

  // Knockback
  position += velocity * dt;
  velocity = velocity * std::exp(-9.0F * dt);

  // Afterimages while dashing
  trail_timer -= dt;
  if (trail_timer <= 0.0F) {
    trail_timer = 0.02F;
    if (is_dashing()) {
      trail.push_back(position);
    }
    if (trail.size() > 6 || (!is_dashing() && !trail.empty())) {
      trail.pop_front();
    }
  }

  // Energy regenerates, faster with regen packages
  const float regen =
      BASE_REGEN + (REGEN_PER_LEVEL * static_cast<float>(regen_level));
  energy = std::min(MAX_STAT, energy + (regen * dt));
}

void Knight::draw(const Camera& camera) const {
  const auto& idle = face_right ? tex_right : tex_left;

  // Dash afterimages
  float trail_alpha = 0.1F;
  for (const auto& p : trail) {
    const auto quad =
        asw::Quadf(p.x - SPRITE_W / 2, p.y - SPRITE_H, SPRITE_W, SPRITE_H);
    asw::draw::set_alpha(idle, trail_alpha);
    asw::draw::stretch_sprite(idle, camera.to_screen(quad));
    trail_alpha += 0.07F;
  }
  asw::draw::set_alpha(idle, 1.0F);

  // Flicker while invulnerable
  if (invulnerable_timer > 0.0F &&
      static_cast<int>(invulnerable_timer * 14.0F) % 2 == 0) {
    return;
  }

  asw::Texture texture = idle;
  if (is_swinging()) {
    texture = face_right ? tex_right_slash : tex_left_slash;
  } else if (hammer_up) {
    texture = face_right ? tex_right_hammer : tex_left_hammer;
  }

  const auto quad = asw::Quadf(position.x - SPRITE_W / 2, position.y - SPRITE_H,
                               SPRITE_W, SPRITE_H);
  asw::draw::stretch_sprite(texture, camera.to_screen(quad));

  // Sword sweep
  if (is_swinging()) {
    const float progress = 1.0F - (swing_timer / SWING_TIME);
    const float base_angle = std::atan2(facing.y, facing.x);
    const float sweep = std::numbers::pi_v<float> * 0.8F;
    const auto origin = camera.to_screen(center());

    constexpr int segments = 9;
    for (int i = 0; i < segments; i++) {
      const float t = static_cast<float>(i) / (segments - 1);
      if (t > progress) {
        break;
      }

      const float angle = base_angle - (sweep / 2.0F) + (sweep * t);
      const auto point =
          origin + asw::Vec2f(std::cos(angle), std::sin(angle)) * 70.0F;
      const auto alpha =
          static_cast<uint8_t>(90.0F + (150.0F * t * (1.0F - progress * 0.6F)));
      asw::draw::circle_fill(point, 7.0F + (6.0F * t),
                             asw::Color(255, 255, 255, alpha));
    }
  }
}

asw::Quadf Knight::body() const {
  return {position.x - 18.0F, position.y - 40.0F, 36.0F, 40.0F};
}

asw::Vec2f Knight::center() const {
  return {position.x, position.y - 52.0F};
}

asw::Vec2f Knight::swing_center() const {
  return center() + facing * 60.0F;
}

bool Knight::try_swing() {
  if (swing_cooldown > 0.0F || is_dashing() || energy < SWING_COST) {
    return false;
  }

  spend_energy(SWING_COST);
  swing_timer = SWING_TIME;
  swing_cooldown = SWING_COOLDOWN;
  return true;
}

bool Knight::try_dash(const asw::Vec2f& direction) {
  if (dash_cooldown > 0.0F || is_dashing() || energy < DASH_COST) {
    return false;
  }

  if (direction.magnitude() > 0.1F) {
    facing = direction / direction.magnitude();
    if (std::abs(direction.x) > 0.1F) {
      face_right = direction.x > 0.0F;
    }
  }

  spend_energy(DASH_COST);
  dash_timer = DASH_TIME;
  dash_cooldown = DASH_COOLDOWN;
  return true;
}

bool Knight::hammer_ready() const {
  return power >= HAMMER_COST && energy >= HAMMER_COST;
}

void Knight::use_hammer() {
  power -= HAMMER_COST;
  spend_energy(HAMMER_COST);
}

bool Knight::hurt(int amount, const asw::Vec2f& from) {
  if (invulnerable_timer > 0.0F || is_dashing()) {
    return false;
  }

  health = std::max(0.0F, health - static_cast<float>(amount));
  invulnerable_timer = INVULNERABLE_TIME;

  // Knock away from the attacker
  auto away = position - from;
  if (away.magnitude() < 0.01F) {
    away = asw::Vec2f(0, 1);
  }
  velocity = (away / away.magnitude()) * 650.0F;

  return true;
}

void Knight::spend_energy(float amount) {
  energy = std::max(0.0F, energy - amount);
  energy_used += amount;
}
