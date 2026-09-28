#pragma once

#include <asw/asw.h>
#include <array>
#include <string>
#include <vector>

#include "../game/effects.h"
#include "../game/entities.h"
#include "../game/knight.h"
#include "../game/level.h"
#include "./state.h"

// Main game screen
class Game : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;
  void cleanup() override;

 private:
  // Setup
  void load_assets();
  void build_level();

  // Simulation
  void step(float dt);
  void update_towers(float dt);
  void update_slimes(float dt);
  void update_packages(float dt);
  void update_combo(float dt);
  void check_end();

  // Actions
  void swing();
  void slam();
  void call_package(int index);
  void spawn_boss();

  // Slimes
  Slime make_slime(SlimeType type, const asw::Vec2f& position) const;
  void hit_slime(Slime& slime, int damage, const asw::Vec2f& from);
  void kill_slime(Slime& slime);
  asw::Vec2f slime_center(const Slime& slime) const;

  // Standing tower closest to the knight, nullptr if none are left
  const Tower* nearest_tower() const;

  // Push a box out of solid scenery, returns the corrected box
  asw::Quadf resolve_solids(asw::Quadf box) const;

  // Pause, win and lose
  void finish(bool won);
  void set_banner(const std::string& title, const std::string& subtitle);

  // Drawing
  void draw_map() const;
  void draw_objects() const;
  void draw_slime(const Slime& slime) const;
  void draw_shadow(const asw::Vec2f& position, float width) const;
  void draw_tower_markers() const;
  void draw_hud() const;
  void draw_minimap() const;
  void draw_banner() const;
  void draw_pause() const;

  const LevelConfig* config{nullptr};

  // Objects
  Knight knight;
  asw::Camera camera{asw::Vec2f(SCREEN_W, SCREEN_H)};
  Effects effects;
  std::vector<Slime> slimes;
  std::vector<Slime> new_slimes;
  std::vector<Tower> towers;
  std::vector<Ruin> ruins;
  std::vector<Package> packages;

  // Level state
  float spawn_timer{0.0F};
  float level_time{0.0F};
  float hitstop{0.0F};
  float finish_timer{0.0F};
  bool gates_open{false};
  bool boss_spawned{false};
  bool paused{false};
  bool finished{false};
  bool won{false};

  // Combo
  int combo{0};
  float combo_timer{0.0F};

  // Stats
  int kills{0};
  int damage_taken{0};
  int power_received{0};
  int best_combo{0};

  // Banner
  std::string banner_title;
  std::string banner_subtitle;
  float banner_timer{0.0F};

  // Assets
  asw::Texture tex_map;
  asw::Texture tex_ruin;
  asw::Texture tex_tower;
  asw::Texture tex_tower_open;
  asw::Texture tex_tower_broken;
  asw::SpriteSheet sheet_slime_green;
  asw::SpriteSheet sheet_slime_red;
  asw::SpriteSheet sheet_slime_purple;
  asw::SpriteSheet sheet_boss;
  asw::Texture tex_slime_death;
  asw::Texture tex_shadow;
  asw::Texture tex_hud;
  asw::Texture tex_bar_health;
  asw::Texture tex_bar_energy;
  asw::Texture tex_bar_power;
  asw::Texture tex_destroy_ready;
  std::array<asw::Texture, 4> tex_parachutes;
  std::array<asw::Texture, 4> tex_boxes;

  asw::Font font_small;
  asw::Font font_medium;
  asw::Font font_big;
};
