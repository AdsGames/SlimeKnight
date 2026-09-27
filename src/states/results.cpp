#include "./results.h"

#include <array>
#include <string>
#include <utility>

#include "../save_data.h"
#include "../game/audio.h"
#include "../globals.h"

void Results::init() {
  background = asw::assets::load_texture("assets/images/ui/results.png");
  font = asw::assets::load_font("assets/fonts/ariblk.ttf", 32);
  timer = 0.0F;
  Audio::play_music("postcarnage");
}

void Results::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);
  timer += dt;

  if (timer < 0.8F) {
    return;
  }

  const bool controller =
      asw::input::get_controller_count() > 0 &&
      asw::input::get_controller_button_down(0,
                                             asw::input::ControllerButton::A);

  if (asw::input::get_key_down(asw::input::Key::Return) ||
      asw::input::get_key_down(asw::input::Key::Space) ||
      asw::input::get_mouse_button_down(asw::input::MouseButton::Left) ||
      controller) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::LevelSelect);
  }
}

void Results::draw() {
  asw::draw::stretch_sprite(background, asw::Quadf(0, 0, SCREEN_W, SCREEN_H));

  const asw::Color ink(0, 0, 0);

  // Values next to the labels baked into the background
  const std::array<std::string, 4> values = {
      std::to_string(last_stats.kills),
      std::to_string(last_stats.damage_taken),
      std::to_string(last_stats.power_received),
      std::to_string(last_stats.energy_used),
  };
  const std::array<float, 4> rows = {184.0F, 232.0F, 283.0F, 333.0F};

  for (std::size_t i = 0; i < values.size(); i++) {
    asw::draw::text(font, values[i], asw::Vec2f(470, rows[i]), ink);
  }

  // Extra stats in the same style
  const float best = SaveData::get_best_time(current_level);
  const std::array<std::pair<std::string, std::string>, 4> extra = {{
      {"Towers Smashed:", std::to_string(last_stats.towers)},
      {"Best Combo:", std::to_string(last_stats.best_combo)},
      {"Time:", format_time(last_stats.time)},
      {"Best Time:", format_time(best)},
  }};

  float y = 383.0F;
  for (const auto& [label, value] : extra) {
    asw::draw::text(font, label, asw::Vec2f(106, y), ink);
    asw::draw::text(font, value, asw::Vec2f(470, y), ink);
    y += 50.0F;
  }

  if (last_stats.new_best) {
    asw::draw::text(font, "New best time!", asw::Vec2f(106, y + 20),
                    asw::Color(255, 240, 80));
  }
}
