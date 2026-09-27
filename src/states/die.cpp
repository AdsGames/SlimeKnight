#include "./die.h"

#include <string>

#include "../game/audio.h"
#include "../globals.h"

void Die::init() {
  background = asw::assets::load_texture("assets/images/ui/lose.png");
  font = asw::assets::load_font("assets/fonts/ariblk.ttf", 26);
  timer = 0.0F;
  Audio::play_music("die");
}

void Die::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);
  timer += dt;

  if (timer < 0.8F) {
    return;
  }

  const bool has_controller = asw::input::get_controller_count() > 0;

  // Retry straight away
  if (asw::input::get_key_down(asw::input::Key::R) ||
      (has_controller && asw::input::get_controller_button_down(
                             0, asw::input::ControllerButton::Y))) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::Game);
    return;
  }

  if (asw::input::get_key_down(asw::input::Key::Return) ||
      asw::input::get_mouse_button_down(asw::input::MouseButton::Left) ||
      (has_controller && asw::input::get_controller_button_down(
                             0, asw::input::ControllerButton::A))) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::LevelSelect);
  }
}

void Die::draw() {
  asw::draw::stretch_sprite(background, asw::Quadf(0, 0, SCREEN_W, SCREEN_H));

  const auto summary =
      "Slimes slain: " + std::to_string(last_stats.kills) +
      "    Towers smashed: " + std::to_string(last_stats.towers) +
      "    Time: " + format_time(last_stats.time);
  asw::draw::text(font, summary, asw::Vec2f(SCREEN_W / 2.0F, 520),
                  asw::Color(0, 0, 0), asw::TextJustify::Center);
  asw::draw::text(font, "Press R to try again",
                  asw::Vec2f(SCREEN_W / 2.0F, 580), asw::Color(0, 0, 0),
                  asw::TextJustify::Center);
}
