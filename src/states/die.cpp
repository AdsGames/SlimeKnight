#include "./die.h"

#include <string>

#include "../controls.h"
#include "../game/audio.h"
#include "../globals.h"
#include "../ui/screenshot.h"

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

  // Retry straight away
  if (asw::input::get_action_down(action::RETRY)) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::Game);
    return;
  }

  if (asw::input::get_action_down(action::CONFIRM) ||
      asw::input::get_mouse_button_down(asw::input::MouseButton::Left)) {
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
  asw::draw::text(font,
                  prompt("Press R to try again",
                         "Press Y to try again, A to continue"),
                  asw::Vec2f(SCREEN_W / 2.0F, 580), asw::Color(0, 0, 0),
                  asw::TextJustify::Center);

  handle_screenshot_key();
}
