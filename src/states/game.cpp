#include "./game.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

#include "../controls.h"
#include "../game/audio.h"
#include "../save_data.h"
#include "../ui/screenshot.h"

namespace {
// Care package costs in power, index matches PackageType
constexpr std::array<float, 4> PACKAGE_COSTS = {80.0F, 40.0F, 40.0F, 60.0F};

// Where the package icons sit on the hud image
constexpr std::array<float, 4> PACKAGE_ICON_X = {340.0F, 420.0F, 500.0F,
                                                 566.0F};
constexpr float PACKAGE_ICON_Y = 118.0F;
constexpr float PACKAGE_ICON_SIZE = 36.0F;

// Hud bars
constexpr float BAR_WIDTH = 292.0F;
const asw::Quadf HEALTH_BAR(12, 44, BAR_WIDTH, 28);
const asw::Quadf ENERGY_BAR(12, 122, BAR_WIDTH, 30);
const asw::Quadf POWER_BAR(332, 44, BAR_WIDTH, 28);

// Minimap
const asw::Quadf MINIMAP(1036, 14, 228, 171);

constexpr std::size_t MAX_SLIMES = 120;
constexpr int MAX_SPEED_LEVEL = 4;
constexpr float SLAM_TOWER_RANGE = 170.0F;
constexpr float SLAM_SLIME_RANGE = 240.0F;
constexpr float COMBO_TIME = 2.2F;
constexpr float BOSS_MAX_RADIUS = 190.0F;
constexpr float BOSS_MIN_RADIUS = 45.0F;
constexpr int BOSS_HEALTH = 40;

// Tint used for effects of each slime colour
asw::Color slime_color(SlimeType type) {
  switch (type) {
    case SlimeType::Green:
      return {40, 220, 60};
    case SlimeType::Red:
      return {220, 30, 30};
    case SlimeType::Purple:
    case SlimeType::Mini:
      return {200, 40, 170};
    case SlimeType::Boss:
      return {30, 80, 220};
  }

  return {40, 220, 60};
}

const asw::Quadf WALK_AREA(WALK_INSET,
                           WALK_INSET,
                           WORLD_W - (WALK_INSET * 2),
                           WORLD_H - (WALK_INSET * 2));
}  // namespace

void Game::init() {
  config = &LEVELS[current_level];

  load_assets();
  build_level();

  Audio::play_music(config->music);
  set_banner("Level " + std::to_string(current_level + 1) + ": " +
                 config->name,
             "Smash the towers and slay every slime!");
}

void Game::cleanup() {
  slimes.clear();
  new_slimes.clear();
  towers.clear();
  ruins.clear();
  packages.clear();
  effects.clear();
  asw::sound::resume_music();
  asw::scene::Scene<ProgramState>::cleanup();
}

void Game::load_assets() {
  const std::string images = "assets/images/";

  tex_map = asw::assets::load_texture(images + "worlds/" + config->map + ".png");
  asw::draw::set_scale_mode(tex_map, asw::ScaleMode::Nearest);

  tex_ruin =
      asw::assets::load_texture(images + "game/" + config->ruin + ".png");
  tex_tower = asw::assets::load_texture(images + "game/tower.png");
  tex_tower_open = asw::assets::load_texture(images + "game/tower_open.png");
  tex_tower_broken =
      asw::assets::load_texture(images + "game/tower_broken.png");
  // Slime sheets are 2x2 frames, sizes match the original animation files
  const asw::Vec2f slime_frame(72.0F, 46.08F);
  sheet_slime_green = asw::SpriteSheet(
      asw::assets::load_texture(images + "game/slime_green.png"), slime_frame);
  sheet_slime_red = asw::SpriteSheet(
      asw::assets::load_texture(images + "game/slime_red.png"), slime_frame);
  sheet_slime_purple = asw::SpriteSheet(
      asw::assets::load_texture(images + "game/slime_purple.png"), slime_frame);
  sheet_boss =
      asw::SpriteSheet(asw::assets::load_texture(images + "game/boss.png"),
                       asw::Vec2f(144.0F, 92.16F));
  tex_slime_death = asw::assets::load_texture(images + "game/slime_death.png");
  tex_hud = asw::assets::load_texture(images + "ui/hud.png");
  tex_bar_health = asw::assets::load_texture(images + "ui/bar_health.png");
  tex_bar_energy = asw::assets::load_texture(images + "ui/bar_energy.png");
  tex_bar_power = asw::assets::load_texture(images + "ui/bar_power.png");
  tex_destroy_ready =
      asw::assets::load_texture(images + "ui/destroy_ready.png");

  tex_parachutes = {
      asw::assets::load_texture(images + "game/parachute_regen.png"),
      asw::assets::load_texture(images + "game/parachute_health.png"),
      asw::assets::load_texture(images + "game/parachute_energy.png"),
      asw::assets::load_texture(images + "game/parachute_speed.png"),
  };
  tex_boxes = {
      asw::assets::load_texture(images + "game/box_regen.png"),
      asw::assets::load_texture(images + "game/box_health.png"),
      asw::assets::load_texture(images + "game/box_energy.png"),
      asw::assets::load_texture(images + "game/box_speed.png"),
  };

  tex_shadow = asw::assets::create_radial_gradient(64, asw::Color(0, 0, 0, 120),
                                                   asw::Color(0, 0, 0, 0));

  font_small = asw::assets::load_font("assets/fonts/ariblk.ttf", 16);
  font_medium = asw::assets::load_font("assets/fonts/ariblk.ttf", 22);
  font_big = asw::assets::load_font("assets/fonts/ariblk.ttf", 44);

  knight.init();
  effects.init(font_small, font_medium);
}

void Game::build_level() {
  slimes.clear();
  new_slimes.clear();
  towers.clear();
  ruins.clear();
  packages.clear();

  const asw::Vec2f start(WORLD_W / 2.0F, WORLD_H / 2.0F);

  // Towers, spread out and away from the start
  int attempts = 0;
  while (static_cast<int>(towers.size()) < config->towers && attempts < 5000) {
    attempts++;
    const asw::Vec2f base(
        asw::random::between(BUILD_INSET, WORLD_W - BUILD_INSET),
        asw::random::between(BUILD_INSET + Tower::HEIGHT,
                             WORLD_H - BUILD_INSET));

    if (base.distance(start) < 520.0F) {
      continue;
    }

    const float spacing = attempts < 3000 ? 420.0F : 260.0F;
    const bool crowded = std::ranges::any_of(towers, [&](const Tower& t) {
      return t.base.distance(base) < spacing;
    });

    if (!crowded) {
      towers.push_back(Tower{base, false});
    }
  }

  // Ruins, never on top of towers, the start or each other
  attempts = 0;
  while (static_cast<int>(ruins.size()) < config->ruins && attempts < 5000) {
    attempts++;
    const asw::Quadf bounds(
        asw::random::between(BUILD_INSET, WORLD_W - BUILD_INSET),
        asw::random::between(BUILD_INSET, WORLD_H - BUILD_INSET), Ruin::SIZE,
        Ruin::SIZE);

    if (bounds.get_center().distance(start) < 260.0F) {
      continue;
    }

    const auto padded = asw::Quadf(bounds.position.x - 50,
                                   bounds.position.y - 50, bounds.size.x + 100,
                                   bounds.size.y + 100);

    const bool blocked =
        std::ranges::any_of(towers,
                            [&](const Tower& t) {
                              return padded.collides(t.sprite_area());
                            }) ||
        std::ranges::any_of(ruins, [&](const Ruin& r) {
          return padded.collides(r.bounds);
        });

    if (!blocked) {
      ruins.push_back(Ruin{bounds});
    }
  }

  knight.reset(start);

  camera.set_bounds(asw::Quadf(0, -210, WORLD_W, WORLD_H + 210));
  camera.set_anchor(asw::Vec2f(SCREEN_W / 2.0F, 600.0F));
  camera.snap_to(knight.center());
  Audio::set_listener(knight.center());

  // First wave comes a little sooner than the rest
  spawn_timer = config->spawn_interval * 0.4F;
  level_time = 0.0F;
  hitstop = 0.0F;
  finish_timer = 0.0F;
  gates_open = false;
  boss_spawned = false;
  paused = false;
  finished = false;
  won = false;
  combo = 0;
  combo_timer = 0.0F;
  kills = 0;
  damage_taken = 0;
  power_received = 0;
  best_combo = 0;
}

void Game::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);

  // Pause
  if (asw::input::get_action_down(action::PAUSE) && !finished) {
    paused = !paused;
    Audio::play(Sfx::Click);
    if (paused) {
      asw::sound::pause_music();
    } else {
      asw::sound::resume_music();
    }
  }

  if (paused) {
    if (asw::input::get_action_down(action::QUIT_LEVEL)) {
      manager.set_next_scene(ProgramState::LevelSelect);
    }
    return;
  }

  // asw calls update in fixed steps on every platform
  step(dt);
}

void Game::step(float dt) {
  banner_timer = std::max(0.0F, banner_timer - dt);
  effects.update(dt);

  // Short freeze on big hits
  if (hitstop > 0.0F) {
    hitstop -= dt;
    return;
  }

  if (finished) {
    finish_timer -= dt;
    camera.follow(knight.center(), dt);
    camera.update(dt);
    update_slimes(dt);

    if (finish_timer <= 0.0F) {
      manager.set_next_scene(won ? ProgramState::Results : ProgramState::Die);
    }
    return;
  }

  level_time += dt;

  // Knight
  const auto input = KnightInput::read();

  if (input.dash && knight.try_dash(input.move)) {
    Audio::play(Sfx::Sword, 0.35F);
    effects.burst(knight.position, asw::Color(200, 200, 200, 180), 10, 160.0F,
                  6.0F);
  }

  if (input.slash) {
    swing();
  }

  if (input.hammer) {
    slam();
  }

  if (input.package >= 0) {
    call_package(input.package);
  }

  knight.update(dt, input);
  Audio::set_listener(knight.center());

  // Keep the knight out of scenery and on the map
  const auto body = resolve_solids(knight.body());
  knight.position =
      asw::Vec2f(body.position.x + (body.size.x / 2.0F),
                 body.position.y + body.size.y);
  knight.position.x =
      std::clamp(knight.position.x, WALK_AREA.position.x,
                 WALK_AREA.position.x + WALK_AREA.size.x);
  knight.position.y =
      std::clamp(knight.position.y, WALK_AREA.position.y,
                 WALK_AREA.position.y + WALK_AREA.size.y);

  update_towers(dt);
  update_slimes(dt);
  update_packages(dt);
  update_combo(dt);

  camera.follow(knight.center(), dt);
  camera.update(dt);

  check_end();
}

void Game::update_towers(float dt) {
  const auto alive = static_cast<float>(
      std::ranges::count_if(towers, [](const Tower& t) { return !t.destroyed; }));

  if (alive <= 0.0F) {
    return;
  }

  // Towers spawn faster as their numbers drop
  const float interval =
      config->spawn_interval *
      (0.55F + (0.45F * alive / static_cast<float>(towers.size())));

  spawn_timer += dt;

  // Gates open just before a wave and close after it, heard from the
  // closest tower
  const auto* nearest = nearest_tower();
  if (!gates_open && spawn_timer >= interval - 0.6F) {
    gates_open = true;
    Audio::play_at(Sfx::GateOpen, nearest->base, 0.5F);
  } else if (gates_open && spawn_timer > 0.5F && spawn_timer < interval - 0.6F) {
    gates_open = false;
    Audio::play_at(Sfx::GateClose, nearest->base, 0.5F);
  }

  if (spawn_timer < interval) {
    return;
  }

  spawn_timer = 0.0F;

  for (const auto& tower : towers) {
    if (tower.destroyed || slimes.size() + new_slimes.size() >= MAX_SLIMES) {
      continue;
    }

    auto type = SlimeType::Green;
    const float roll = asw::random::between(0.0F, 1.0F);
    if (roll < config->purple_chance) {
      type = SlimeType::Purple;
    } else if (roll < config->purple_chance + config->red_chance) {
      type = SlimeType::Red;
    }

    new_slimes.push_back(make_slime(type, tower.base + asw::Vec2f(0, 24)));
  }

  // One spawn sound per wave, from the closest tower
  Audio::play_at(
      asw::random::chance() ? Sfx::SlimeSpawn1 : Sfx::SlimeSpawn2,
      nearest->base, 0.8F);
}

const Tower* Game::nearest_tower() const {
  const Tower* nearest = nullptr;
  float best = 0.0F;

  for (const auto& tower : towers) {
    const float distance = tower.base.distance(knight.position);
    if (!tower.destroyed && (nearest == nullptr || distance < best)) {
      nearest = &tower;
      best = distance;
    }
  }

  return nearest;
}

Slime Game::make_slime(SlimeType type, const asw::Vec2f& position) const {
  Slime slime;
  slime.type = type;
  slime.position = position;
  slime.spawn_timer = 0.35F;
  // Start at a random point so slimes do not wobble in step
  slime.animation.update(asw::random::between(0.0F, 1.0F));

  switch (type) {
    case SlimeType::Green:
      slime.radius = 27.0F;
      slime.hop_period = 0.95F;
      slime.hop_speed = 330.0F;
      break;
    case SlimeType::Red:
      slime.radius = 25.0F;
      slime.hop_period = 0.6F;
      slime.hop_speed = 430.0F;
      break;
    case SlimeType::Purple:
      slime.radius = 33.0F;
      slime.health = 2;
      slime.hop_period = 1.15F;
      slime.hop_speed = 300.0F;
      break;
    case SlimeType::Mini:
      slime.radius = 17.0F;
      slime.hop_period = 0.55F;
      slime.hop_speed = 380.0F;
      break;
    case SlimeType::Boss:
      slime.radius = BOSS_MAX_RADIUS;
      slime.health = BOSS_HEALTH;
      slime.spawn_timer = 1.2F;
      slime.charge_timer = 4.0F;
      break;
  }

  slime.max_health = slime.health;
  slime.hop_timer = asw::random::between(0.1F, slime.hop_period);
  return slime;
}

void Game::update_slimes(float dt) {
  // Add slimes created last step
  slimes.insert(slimes.end(), new_slimes.begin(), new_slimes.end());
  new_slimes.clear();

  for (auto& slime : slimes) {
    slime.animation.update(dt);
    slime.hurt_timer = std::max(0.0F, slime.hurt_timer - dt);

    if (slime.spawn_timer > 0.0F) {
      slime.spawn_timer -= dt;
      continue;
    }

    const auto to_knight = (knight.position - slime.position).normalized();

    if (slime.type == SlimeType::Boss) {
      // Boss creeps forward and charges now and then
      if (slime.telegraph_timer > 0.0F) {
        slime.telegraph_timer -= dt;
        if (slime.telegraph_timer <= 0.0F) {
          slime.velocity = to_knight * 900.0F;
          camera.shake(10.0F);
          Audio::play_at(Sfx::Splat, slime.position, 0.8F);
          slime.charge_timer = asw::random::between(3.0F, 4.5F);
        }
      } else {
        slime.charge_timer -= dt;
        slime.position += to_knight * (85.0F * dt);
        if (slime.charge_timer <= 0.0F) {
          slime.telegraph_timer = 0.8F;
        }
      }
    } else {
      // Hop towards the knight with a bit of wobble
      slime.hop_timer -= dt;
      if (slime.hop_timer <= 0.0F) {
        slime.hop_timer =
            slime.hop_period * asw::random::between(0.8F, 1.2F);
        const float wobble = asw::random::between(-0.45F, 0.45F);
        const float angle = std::atan2(to_knight.y, to_knight.x) + wobble;
        slime.velocity =
            asw::Vec2f(std::cos(angle), std::sin(angle)) * slime.hop_speed;
      }
    }

    slime.position += slime.velocity * dt;
    slime.velocity = slime.velocity * std::exp(-3.5F * dt);

    // Scenery
    if (slime.type != SlimeType::Boss) {
      const float half = slime.radius * 0.7F;
      const auto box = resolve_solids(asw::Quadf(
          slime.position.x - half, slime.position.y - half, half * 2, half * 2));
      slime.position = box.get_center();
    }

    slime.position.x =
        std::clamp(slime.position.x, WALK_AREA.position.x,
                   WALK_AREA.position.x + WALK_AREA.size.x);
    slime.position.y =
        std::clamp(slime.position.y, WALK_AREA.position.y,
                   WALK_AREA.position.y + WALK_AREA.size.y);

    // Touching the knight hurts
    if (!finished && !knight.is_dead()) {
      if (knight.body().collides(slime.hitbox()) &&
          knight.hurt(slime.damage(), slime.position)) {
        damage_taken += slime.damage();
        combo = 0;
        Audio::play(Sfx::Hurt, 0.8F);
        camera.shake(9.0F);
        effects.text(knight.center() + asw::Vec2f(0, -40),
                     "-" + std::to_string(slime.damage()),
                     asw::Color(230, 40, 40));
        effects.burst(knight.center(), asw::Color(200, 30, 30), 10, 220.0F);
        slime.velocity = to_knight * -380.0F;
      }
    }
  }

  // Keep slimes from stacking on each other
  for (std::size_t i = 0; i < slimes.size(); i++) {
    for (std::size_t j = i + 1; j < slimes.size(); j++) {
      auto& a = slimes[i];
      auto& b = slimes[j];
      const auto diff = b.position - a.position;
      const float min_dist = (a.radius + b.radius) * 0.85F;
      const float dist_sq = (diff.x * diff.x) + (diff.y * diff.y);

      if (dist_sq >= min_dist * min_dist || dist_sq < 0.0001F) {
        continue;
      }

      const float dist = std::sqrt(dist_sq);
      const auto push = (diff / dist) * ((min_dist - dist) * 0.5F);

      // The boss shoves everyone else around
      if (a.type == SlimeType::Boss) {
        b.position += push * 2.0F;
      } else if (b.type == SlimeType::Boss) {
        a.position -= push * 2.0F;
      } else {
        a.position -= push;
        b.position += push;
      }
    }
  }

  std::erase_if(slimes, [](const Slime& s) { return !s.alive; });
}

void Game::update_packages(float dt) {
  for (auto& package : packages) {
    if (!package.landed) {
      package.fall_timer += dt;
      if (package.fall_timer >= Package::FALL_TIME) {
        package.landed = true;
        package.wind.stop(0.4F);
        Audio::play_at(Sfx::Drop, package.target, 0.8F);
        effects.burst(package.target, asw::Color(180, 150, 90), 14, 180.0F);
      }
      continue;
    }

    if (!knight.body().collides(package.box())) {
      continue;
    }

    package.collected = true;
    Audio::play(Sfx::Drop);
    const auto where = knight.center() + asw::Vec2f(0, -50);

    switch (package.type) {
      case PackageType::Regen:
        knight.regen_level =
            std::min(Knight::MAX_REGEN_LEVEL, knight.regen_level + 1);
        effects.text(where, "Energy regen up!", asw::Color(90, 170, 255),
                     true);
        break;
      case PackageType::Health:
        knight.health = std::min(Knight::MAX_STAT, knight.health + 50.0F);
        effects.text(where, "+50 health", asw::Color(240, 60, 60), true);
        break;
      case PackageType::Energy:
        knight.energy = std::min(Knight::MAX_STAT, knight.energy + 60.0F);
        effects.text(where, "+60 energy", asw::Color(90, 170, 255), true);
        break;
      case PackageType::Speed:
        knight.speed_level = std::min(MAX_SPEED_LEVEL, knight.speed_level + 1);
        effects.text(where, "Speed up!", asw::Color(255, 200, 40), true);
        break;
    }

    effects.burst(knight.center(), asw::Color(255, 240, 120), 20, 260.0F);
  }

  std::erase_if(packages, [](const Package& p) { return p.collected; });
}

void Game::update_combo(float dt) {
  if (combo <= 0) {
    return;
  }

  combo_timer -= dt;
  if (combo_timer <= 0.0F) {
    combo = 0;
  }
}

void Game::check_end() {
  if (knight.is_dead()) {
    effects.burst(knight.center(), asw::Color(160, 160, 170), 40, 380.0F, 8.0F);
    effects.burst(knight.center(), asw::Color(200, 20, 20), 30, 300.0F, 7.0F);
    camera.shake(25.0F);
    finish(false);
    return;
  }

  const bool towers_down = std::ranges::all_of(
      towers, [](const Tower& t) { return t.destroyed; });

  if (!towers_down || !slimes.empty() || !new_slimes.empty()) {
    return;
  }

  if (config->boss && !boss_spawned) {
    spawn_boss();
    return;
  }

  finish(true);
}

void Game::swing() {
  if (!knight.try_swing()) {
    return;
  }

  Audio::play(Sfx::Sword, 0.45F);

  const auto center = knight.swing_center();
  for (auto& slime : slimes) {
    if (slime.spawn_timer > 0.0F && slime.type == SlimeType::Boss) {
      continue;
    }

    if (slime.hitbox().distance_to(center) <= Knight::SWING_RADIUS * 0.6F) {
      hit_slime(slime, 1, knight.center());
    }
  }
}

void Game::slam() {
  if (!knight.hammer_ready()) {
    effects.text(knight.center() + asw::Vec2f(0, -70),
                 "Hammer needs 30 power + 30 energy",
                 asw::Color(255, 255, 255));
    Audio::play(Sfx::Click, 0.6F);
    return;
  }

  knight.use_hammer();

  const auto origin = knight.position;
  effects.ring(origin, SLAM_SLIME_RANGE, asw::Color(140, 110, 70, 220));
  camera.shake(14.0F);
  hitstop = 0.05F;
  Audio::play(Sfx::TowerDestroy, 0.5F);

  // Towers in range crumble
  for (auto& tower : towers) {
    if (tower.destroyed ||
        tower.footprint().distance_to(origin) > SLAM_TOWER_RANGE) {
      continue;
    }

    tower.destroyed = true;
    hitstop = 0.12F;
    camera.shake(26.0F);

    // Let the crash cut through the music
    const auto middle = tower.sprite_area().get_center();
    Audio::play_at(Sfx::TowerDestroy, middle);
    Audio::duck_music(0.35F, 0.6F);

    effects.debris(middle, tower.base.y, asw::Color(70, 70, 75), 28);
    effects.burst(middle, asw::Color(120, 110, 100, 160), 20, 300.0F, 14.0F);
    effects.burst(middle, asw::Color(40, 220, 60), 25, 420.0F, 8.0F, 300.0F);
    effects.text(middle, "TOWER DESTROYED!", asw::Color(255, 220, 60), true);
  }

  // Slimes nearby are flattened
  for (auto& slime : slimes) {
    if (slime.hitbox().distance_to(origin) <= SLAM_SLIME_RANGE) {
      hit_slime(slime, slime.type == SlimeType::Boss ? 4 : 3, origin);
    }
  }
}

void Game::call_package(int index) {
  const auto where = knight.center() + asw::Vec2f(0, -70);
  const auto type = static_cast<PackageType>(index);

  if (type == PackageType::Regen &&
      knight.regen_level >= Knight::MAX_REGEN_LEVEL) {
    effects.text(where, "Regen is maxed out", asw::Color(255, 255, 255));
    Audio::play(Sfx::Click, 0.6F);
    return;
  }

  if (knight.power < PACKAGE_COSTS[index]) {
    effects.text(where,
                 "Need " + std::to_string(static_cast<int>(PACKAGE_COSTS[index])) +
                     " power",
                 asw::Color(255, 255, 255));
    Audio::play(Sfx::Click, 0.6F);
    return;
  }

  knight.power -= PACKAGE_COSTS[index];

  Package package;
  package.type = type;
  package.target = knight.position + (knight.facing * 90.0F);
  package.target.x =
      std::clamp(package.target.x, WALK_AREA.position.x,
                 WALK_AREA.position.x + WALK_AREA.size.x);
  package.target.y =
      std::clamp(package.target.y, WALK_AREA.position.y + 60.0F,
                 WALK_AREA.position.y + WALK_AREA.size.y);
  package.wind = Audio::play_at(Sfx::Wind, package.target, 0.5F);
  packages.push_back(package);
}

void Game::spawn_boss() {
  boss_spawned = true;

  // Drop in above the knight, inside the map
  auto position = knight.position + asw::Vec2f(0, -380);
  position.y = std::max(position.y, WALK_AREA.position.y + 200.0F);

  slimes.push_back(make_slime(SlimeType::Boss, position));

  Audio::play_music("boss");
  Audio::play_at(Sfx::SlimeSpawn1, position);
  camera.shake(20.0F);
  set_banner("The Slime King awakens!", "Every hit makes him smaller...");
}

void Game::hit_slime(Slime& slime, int damage, const asw::Vec2f& from) {
  if (!slime.alive) {
    return;
  }

  slime.health -= damage;
  slime.hurt_timer = 0.15F;

  const auto away = (slime.position - from).normalized();
  const auto center = slime_center(slime);

  if (slime.health <= 0) {
    kill_slime(slime);
    return;
  }

  if (slime.type == SlimeType::Boss) {
    // Boss shrinks as it is hurt and sheds minions
    const float t = static_cast<float>(slime.health) /
                    static_cast<float>(slime.max_health);
    slime.radius = BOSS_MIN_RADIUS + ((BOSS_MAX_RADIUS - BOSS_MIN_RADIUS) * t);
    slime.velocity = away * 180.0F;
    camera.shake(5.0F);
    knight.power = std::min(Knight::MAX_STAT, knight.power + 1.0F);
    power_received += 1;

    if (asw::random::chance(0.5F)) {
      auto minion = make_slime(
          asw::random::chance() ? SlimeType::Green : SlimeType::Red,
          slime.position);
      const float angle =
          asw::random::between(0.0F, 2.0F * std::numbers::pi_v<float>);
      minion.velocity = asw::Vec2f(std::cos(angle), std::sin(angle)) * 500.0F;
      minion.spawn_timer = 0.15F;
      new_slimes.push_back(minion);
    }
  } else {
    slime.velocity = away * 520.0F;
  }

  Audio::play_at(Sfx::SlimeHit, center, 0.7F);
  effects.burst(center, slime_color(slime.type), 8, 220.0F);
}

void Game::kill_slime(Slime& slime) {
  slime.alive = false;
  const auto center = slime_center(slime);
  const auto color = slime_color(slime.type);

  // Splat on the ground
  const float size = slime.radius * 2.2F;
  effects.decal(tex_slime_death,
                asw::Quadf(slime.position.x - size / 2,
                           slime.position.y - size * 0.7F, size, size));
  effects.burst(center, color, slime.type == SlimeType::Boss ? 120 : 18,
                slime.type == SlimeType::Boss ? 700.0F : 320.0F,
                slime.type == SlimeType::Boss ? 12.0F : 6.0F);
  Audio::play_at(Sfx::Splat, center, 0.7F);

  kills++;

  // Chain kills for a combo, which gives more power
  combo++;
  combo_timer = COMBO_TIME;
  best_combo = std::max(best_combo, combo);

  int gain = slime.type == SlimeType::Mini ? 2 : 4;
  gain += std::min(combo - 1, 6);
  if (slime.type == SlimeType::Boss) {
    gain = 0;
  }

  if (gain > 0) {
    knight.power =
        std::min(Knight::MAX_STAT, knight.power + static_cast<float>(gain));
    power_received += gain;
    effects.text(center + asw::Vec2f(0, -20), "+" + std::to_string(gain),
                 asw::Color(255, 220, 60));
  }

  if (combo >= 5 && combo % 5 == 0) {
    effects.text(knight.center() + asw::Vec2f(0, -90),
                 std::to_string(combo) + " COMBO!", asw::Color(255, 120, 30),
                 true);
  }

  // Purple slimes split in two
  if (slime.type == SlimeType::Purple) {
    for (int i = 0; i < 2; i++) {
      auto mini = make_slime(SlimeType::Mini, slime.position);
      const float side = i == 0 ? -1.0F : 1.0F;
      mini.velocity = asw::Vec2f(side * 380.0F, asw::random::between(-120.0F, 120.0F));
      mini.spawn_timer = 0.1F;
      new_slimes.push_back(mini);
    }
  }

  if (slime.type == SlimeType::Boss) {
    camera.shake(30.0F);
    hitstop = 0.25F;
    Audio::play(Sfx::TowerDestroy);
    Audio::duck_music(0.2F, 1.5F);
    effects.text(center, "THE SLIME KING IS DEAD!", asw::Color(255, 220, 60),
                 true);
  }
}

asw::Vec2f Game::slime_center(const Slime& slime) const {
  return slime.position - asw::Vec2f(0, slime.draw_size().y / 2.0F);
}

asw::Quadf Game::resolve_solids(asw::Quadf box) const {
  for (const auto& ruin : ruins) {
    box.position += box.get_push_out(ruin.bounds);
  }

  for (const auto& tower : towers) {
    if (!tower.destroyed) {
      box.position += box.get_push_out(tower.footprint());
    }
  }

  return box;
}

void Game::finish(bool won) {
  finished = true;
  this->won = won;
  finish_timer = won ? 3.0F : 2.2F;

  last_stats.kills = kills;
  last_stats.damage_taken = damage_taken;
  last_stats.power_received = power_received;
  last_stats.energy_used = static_cast<int>(knight.energy_used);
  last_stats.best_combo = best_combo;
  last_stats.towers = static_cast<int>(std::ranges::count_if(
      towers, [](const Tower& t) { return t.destroyed; }));
  last_stats.time = level_time;
  last_stats.new_best = false;

  if (won) {
    last_stats.new_best = SaveData::record_clear(current_level, level_time);
    set_banner("Level Clear!", "Time " + format_time(level_time));
  } else {
    asw::sound::stop_music(1.5F);
    set_banner("You have fallen...", "");
  }
}

void Game::set_banner(const std::string& title, const std::string& subtitle) {
  banner_title = title;
  banner_subtitle = subtitle;
  banner_timer = 3.0F;
}

// ---- Drawing ----

void Game::draw() {
  asw::draw::clear_color(asw::Color(10, 20, 60));

  draw_map();
  effects.draw_ground(camera);
  draw_objects();
  effects.draw_top(camera);
  draw_tower_markers();
  draw_hud();
  draw_banner();

  if (paused) {
    draw_pause();
  }

  handle_screenshot_key();
}

void Game::draw_map() const {
  // Only blit the part of the map that is on screen
  const auto view = camera.get_view();
  const float top = std::max(0.0F, view.position.y);
  const float bottom = std::min(WORLD_H, view.position.y + view.size.y);
  if (bottom <= top) {
    return;
  }

  const auto tex_size = asw::util::get_texture_size(tex_map);
  const float sx = tex_size.x / WORLD_W;
  const float sy = tex_size.y / WORLD_H;

  const asw::Quadf world(view.position.x, top, view.size.x, bottom - top);
  const asw::Quadf source(world.position.x * sx, world.position.y * sy,
                          world.size.x * sx, world.size.y * sy);

  asw::draw::stretch_sprite_blit(tex_map, source, camera.world_to_screen(world));
}

void Game::draw_shadow(const asw::Vec2f& position, float width) const {
  const float height = width * 0.35F;
  const auto screen = camera.world_to_screen(position);
  asw::draw::stretch_sprite(
      tex_shadow,
      asw::Quadf(screen.x - width / 2, screen.y - height / 2, width, height));
}

void Game::draw_objects() const {
  // Everything is drawn back to front by its base
  enum class Kind { Ruin, Tower, Slime, Knight, Package };
  struct Item {
    float y;
    Kind kind;
    std::size_t index;
  };

  const auto view = camera.get_view();
  const asw::Quadf padded(view.position.x - 300, view.position.y - 300,
                          view.size.x + 600, view.size.y + 600);

  std::vector<Item> items;
  items.reserve(ruins.size() + towers.size() + slimes.size() + 8);

  for (std::size_t i = 0; i < ruins.size(); i++) {
    if (padded.collides(ruins[i].bounds)) {
      items.push_back(
          {ruins[i].bounds.position.y + ruins[i].bounds.size.y, Kind::Ruin, i});
    }
  }
  for (std::size_t i = 0; i < towers.size(); i++) {
    if (padded.contains(towers[i].base)) {
      items.push_back({towers[i].base.y, Kind::Tower, i});
    }
  }
  for (std::size_t i = 0; i < slimes.size(); i++) {
    if (padded.contains(slimes[i].position) ||
        slimes[i].type == SlimeType::Boss) {
      items.push_back({slimes[i].position.y, Kind::Slime, i});
    }
  }
  for (std::size_t i = 0; i < packages.size(); i++) {
    if (packages[i].landed) {
      items.push_back({packages[i].target.y, Kind::Package, i});
    }
  }
  if (!(finished && !won)) {
    items.push_back({knight.position.y, Kind::Knight, 0});
  }

  std::ranges::stable_sort(items, {}, &Item::y);

  for (const auto& item : items) {
    switch (item.kind) {
      case Kind::Ruin:
        asw::draw::stretch_sprite(tex_ruin,
                                  camera.world_to_screen(ruins[item.index].bounds));
        break;

      case Kind::Tower: {
        const auto& tower = towers[item.index];
        draw_shadow(tower.base, 190.0F);
        const auto& texture = tower.destroyed
                                  ? tex_tower_broken
                                  : (gates_open ? tex_tower_open : tex_tower);
        asw::draw::stretch_sprite(texture,
                                  camera.world_to_screen(tower.sprite_area()));
        break;
      }

      case Kind::Slime:
        draw_slime(slimes[item.index]);
        break;

      case Kind::Knight:
        draw_shadow(knight.position, 64.0F);
        knight.draw(camera);
        break;

      case Kind::Package: {
        const auto& package = packages[item.index];
        auto box = package.box();
        box.position.y += std::sin(level_time * 4.0F) * 3.0F;
        draw_shadow(package.target, 44.0F);
        asw::draw::stretch_sprite(tex_boxes[static_cast<int>(package.type)],
                                  camera.world_to_screen(box));
        break;
      }
    }
  }

  // Falling packages are above everything
  for (const auto& package : packages) {
    if (package.landed) {
      continue;
    }

    const float t = package.fall_timer / Package::FALL_TIME;
    draw_shadow(package.target, 20.0F + (30.0F * t));

    const float height = (1.0F - t) * 560.0F;
    const float sway = std::sin(package.fall_timer * 5.0F) * 18.0F * (1.0F - t);
    const asw::Quadf chute(package.target.x - 45.0F + sway,
                           package.target.y - 120.0F - height, 90.0F, 120.0F);
    asw::draw::stretch_sprite(tex_parachutes[static_cast<int>(package.type)],
                              camera.world_to_screen(chute));
  }
}

void Game::draw_slime(const Slime& slime) const {
  const asw::SpriteSheet* sheet = &sheet_slime_green;

  switch (slime.type) {
    case SlimeType::Green:
      sheet = &sheet_slime_green;
      break;
    case SlimeType::Red:
      sheet = &sheet_slime_red;
      break;
    case SlimeType::Purple:
    case SlimeType::Mini:
      sheet = &sheet_slime_purple;
      break;
    case SlimeType::Boss:
      sheet = &sheet_boss;
      break;
  }

  auto size = slime.draw_size();

  // Pop out of the tower
  if (slime.spawn_timer > 0.0F) {
    const float pop = slime.type == SlimeType::Boss ? 1.2F : 0.35F;
    const float t = 1.0F - (slime.spawn_timer / pop);
    size = size * std::clamp(0.2F + (0.8F * t), 0.2F, 1.0F);
  }

  // Squash and stretch while hopping
  const float stretch =
      std::clamp(slime.velocity.magnitude() / 500.0F, 0.0F, 1.0F);
  size.x *= 1.0F - (0.18F * stretch);
  size.y *= 1.0F + (0.3F * stretch);

  // Boss shakes before it charges
  auto base = slime.position;
  if (slime.telegraph_timer > 0.0F) {
    base.x += asw::random::between(-5.0F, 5.0F);
  }

  draw_shadow(slime.position, size.x * 0.9F);

  const asw::Quadf dest(base.x - size.x / 2, base.y - size.y, size.x, size.y);

  const bool flash = slime.hurt_timer > 0.0F ||
                     (slime.telegraph_timer > 0.0F &&
                      static_cast<int>(slime.telegraph_timer * 10.0F) % 2 == 0);
  if (flash) {
    asw::draw::set_tint(sheet->get_texture(), asw::Color(255, 90, 90));
  }

  sheet->draw_frame(slime.animation.get_frame(), camera.world_to_screen(dest));

  if (flash) {
    asw::draw::set_tint(sheet->get_texture(), asw::Color(255, 255, 255));
  }

}

void Game::draw_tower_markers() const {
  // Point to towers that are off screen
  const asw::Vec2f center(SCREEN_W / 2.0F, 600.0F);
  const asw::Quadf area(30, 250, SCREEN_W - 60, SCREEN_H - 280);
  const auto view = camera.get_view();

  for (const auto& tower : towers) {
    if (tower.destroyed || view.collides(tower.sprite_area())) {
      continue;
    }

    const auto screen = camera.world_to_screen(tower.base + asw::Vec2f(0, -100));
    const auto dir = (screen - center).normalized();

    // Walk out from the middle until the edge of the area
    const float tx = dir.x > 0 ? (area.position.x + area.size.x - center.x) / dir.x
                     : dir.x < 0 ? (area.position.x - center.x) / dir.x
                                 : 1e9F;
    const float ty = dir.y > 0 ? (area.position.y + area.size.y - center.y) / dir.y
                     : dir.y < 0 ? (area.position.y - center.y) / dir.y
                                 : 1e9F;
    const auto point = center + dir * std::min(tx, ty);

    const float pulse = 0.6F + (0.4F * std::sin(level_time * 6.0F));
    asw::draw::circle_fill(point, 11.0F, asw::Color(0, 0, 0, 160));
    asw::draw::circle_fill(point, 8.0F,
                           asw::Color(230, 40, 40,
                                      static_cast<uint8_t>(255.0F * pulse)));
    asw::draw::line(point, point + dir * 20.0F, asw::Color(230, 40, 40));
  }
}

void Game::draw_hud() const {
  asw::draw::sprite(tex_hud, asw::Vec2f(0, 0));

  // Bars
  const auto bar = [](const asw::Texture& texture, const asw::Quadf& area,
                      float value) {
    const float fill = std::clamp(value / Knight::MAX_STAT, 0.0F, 1.0F);
    if (fill > 0.0F) {
      asw::draw::stretch_sprite(
          texture, asw::Quadf(area.position, asw::Vec2f(area.size.x * fill,
                                                         area.size.y)));
    }
  };

  bar(tex_bar_health, HEALTH_BAR, knight.health);
  bar(tex_bar_energy, ENERGY_BAR, knight.energy);
  bar(tex_bar_power, POWER_BAR, knight.power);

  // Flash health when low
  if (knight.health < 30.0F && static_cast<int>(level_time * 4.0F) % 2 == 0) {
    asw::draw::rect(HEALTH_BAR, asw::Color(255, 0, 0));
  }

  const auto value_text = [this](const asw::Quadf& area, float value) {
    asw::draw::text(font_small, std::to_string(static_cast<int>(value)),
                    asw::Vec2f(area.position.x + area.size.x - 6,
                               area.position.y + 3),
                    asw::Color(0, 0, 0), asw::TextJustify::Right);
  };

  value_text(HEALTH_BAR, knight.health);
  value_text(ENERGY_BAR, knight.energy);
  value_text(POWER_BAR, knight.power);

  // Care package costs, with lines from the power bar to their icons
  for (std::size_t i = 0; i < PACKAGE_COSTS.size(); i++) {
    const float x = POWER_BAR.position.x +
                    (POWER_BAR.size.x * PACKAGE_COSTS[i] / Knight::MAX_STAT);
    const float bar_bottom = POWER_BAR.position.y + POWER_BAR.size.y;
    asw::draw::rect_fill(asw::Quadf(x - 1, POWER_BAR.position.y, 2,
                                    POWER_BAR.size.y + 6),
                         asw::Color(60, 30, 0));
    asw::draw::text(font_small,
                    std::to_string(static_cast<int>(PACKAGE_COSTS[i])),
                    asw::Vec2f(x, bar_bottom + 4), asw::Color(60, 30, 0),
                    asw::TextJustify::Center);

    const bool maxed = i == 0 && knight.regen_level >= Knight::MAX_REGEN_LEVEL;
    if (knight.power < PACKAGE_COSTS[i] || maxed) {
      asw::draw::rect_fill(asw::Quadf(PACKAGE_ICON_X[i], PACKAGE_ICON_Y,
                                      PACKAGE_ICON_SIZE, PACKAGE_ICON_SIZE),
                           asw::Color(60, 40, 0, 150));
    }
  }

  // Hammer ready
  if (knight.hammer_ready()) {
    const float pulse = 4.0F * std::sin(level_time * 8.0F);
    asw::draw::stretch_sprite(
        tex_destroy_ready,
        asw::Quadf(640 - pulse / 2, 64 - pulse / 2, 84 + pulse, 84 + pulse));
    asw::draw::text(font_small, prompt("E", "Y"), asw::Vec2f(716, 128),
                    asw::Color(0, 0, 0));
  }

  // Icons show number keys, name the d-pad direction for controllers
  if (using_controller()) {
    constexpr std::array<const char*, 4> directions = {"Up", "Left", "Right",
                                                       "Down"};
    for (std::size_t i = 0; i < directions.size(); i++) {
      asw::draw::text(font_small, directions[i],
                      asw::Vec2f(PACKAGE_ICON_X[i] + PACKAGE_ICON_SIZE / 2,
                                 PACKAGE_ICON_Y + PACKAGE_ICON_SIZE + 4),
                      asw::Color(60, 30, 0), asw::TextJustify::Center);
    }
  }

  // Level info
  const int towers_left = static_cast<int>(std::ranges::count_if(
      towers, [](const Tower& t) { return !t.destroyed; }));
  const asw::Color ink(40, 25, 0);
  asw::draw::text(font_medium,
                  "Towers  " + std::to_string(towers_left) + "/" +
                      std::to_string(towers.size()),
                  asw::Vec2f(770, 24), ink);
  asw::draw::text(font_medium,
                  "Slimes  " + std::to_string(slimes.size()),
                  asw::Vec2f(770, 56), ink);
  asw::draw::text(font_medium, "Time  " + format_time(level_time),
                  asw::Vec2f(770, 88), ink);

  if (combo >= 2) {
    const float t = std::clamp(combo_timer / COMBO_TIME, 0.0F, 1.0F);
    asw::draw::text(font_medium, "Combo x" + std::to_string(combo),
                    asw::Vec2f(770, 124), asw::Color(200, 40, 0));
    asw::draw::rect_fill(asw::Quadf(772, 154, 150 * t, 6),
                         asw::Color(200, 40, 0));
  }

  draw_minimap();

  // Boss health, fixed under the hud
  for (const auto& slime : slimes) {
    if (slime.type != SlimeType::Boss || slime.spawn_timer > 0.0F) {
      continue;
    }

    const float width = 500.0F;
    const float x = (SCREEN_W - width) / 2.0F;
    const float fill = static_cast<float>(slime.health) /
                       static_cast<float>(slime.max_health);
    asw::draw::rect_fill(asw::Quadf(x - 3, 897, width + 6, 24),
                         asw::Color(0, 0, 0, 200));
    asw::draw::rect_fill(asw::Quadf(x, 900, width * fill, 18),
                         asw::Color(60, 110, 255));
    asw::draw::text(font_small, "The Slime King",
                    asw::Vec2f(SCREEN_W / 2.0F, 872), asw::Color(255, 255, 255),
                    asw::TextJustify::Center);
  }
}

void Game::draw_minimap() const {
  const float sx = MINIMAP.size.x / WORLD_W;
  const float sy = MINIMAP.size.y / WORLD_H;
  const auto to_map = [&](const asw::Vec2f& p) {
    return asw::Vec2f(MINIMAP.position.x + (p.x * sx),
                      MINIMAP.position.y + (p.y * sy));
  };

  asw::draw::rect_fill(MINIMAP, asw::Color(20, 30, 20, 190));
  asw::draw::rect(MINIMAP, asw::Color(60, 40, 0));

  for (const auto& ruin : ruins) {
    const auto p = to_map(ruin.bounds.position);
    asw::draw::rect_fill(asw::Quadf(p.x, p.y, 3, 3),
                         asw::Color(120, 120, 120, 200));
  }

  for (const auto& tower : towers) {
    const auto p = to_map(tower.base);
    asw::draw::rect_fill(asw::Quadf(p.x - 4, p.y - 6, 8, 8),
                         tower.destroyed ? asw::Color(90, 90, 90)
                                         : asw::Color(230, 40, 40));
  }

  for (const auto& slime : slimes) {
    const auto p = to_map(slime.position);
    if (slime.type == SlimeType::Boss) {
      asw::draw::circle_fill(p, 7.0F, asw::Color(60, 110, 255));
    } else {
      asw::draw::rect_fill(asw::Quadf(p.x - 1, p.y - 1, 3, 3),
                           slime_color(slime.type));
    }
  }

  for (const auto& package : packages) {
    const auto p = to_map(package.target);
    asw::draw::rect_fill(asw::Quadf(p.x - 2, p.y - 2, 5, 5),
                         asw::Color(255, 220, 60));
  }

  // View and knight
  const auto view = camera.get_view();
  const auto view_pos = to_map(view.position);
  asw::draw::rect(asw::Quadf(view_pos.x, view_pos.y, view.size.x * sx,
                             view.size.y * sy),
                  asw::Color(255, 255, 255, 110));

  const auto k = to_map(knight.position);
  asw::draw::circle_fill(k, 3.5F, asw::Color(255, 255, 255));
}

void Game::draw_banner() const {
  if (banner_timer <= 0.0F) {
    return;
  }

  const float fade = std::clamp(banner_timer / 0.6F, 0.0F, 1.0F);
  const auto alpha = static_cast<uint8_t>(255.0F * fade);
  const float y = 420.0F;

  asw::draw::rect_fill(asw::Quadf(0, y - 20, SCREEN_W, 120),
                       asw::Color(0, 0, 0, static_cast<uint8_t>(140.0F * fade)));
  asw::draw::text_shadow(font_big, banner_title, asw::Vec2f(SCREEN_W / 2.0F, y),
                         asw::Color(255, 220, 60, alpha), asw::Color(0, 0, 0),
                         asw::Vec2f(3, 3), asw::TextJustify::Center);
  asw::draw::text(font_medium, banner_subtitle,
                  asw::Vec2f(SCREEN_W / 2.0F, y + 62),
                  asw::Color(255, 255, 255, alpha), asw::TextJustify::Center);
}

void Game::draw_pause() const {
  asw::draw::rect_fill(asw::Quadf(0, 0, SCREEN_W, SCREEN_H),
                       asw::Color(0, 0, 0, 170));
  asw::draw::text(font_big, "Paused", asw::Vec2f(SCREEN_W / 2.0F, 330),
                  asw::Color(255, 220, 60), asw::TextJustify::Center);

  const std::array<const char*, 8> keyboard_lines = {
      "WASD / Arrows  -  Move",
      "Space / J  -  Swing sword (hold to keep swinging)",
      "Shift / K  -  Dash through slimes",
      "E / L  -  Hammer slam (smashes towers)",
      "1 - 4  -  Call in care packages",
      "",
      "Esc / P  -  Resume",
      "Q  -  Quit to level select",
  };
  const std::array<const char*, 8> controller_lines = {
      "Left stick  -  Move",
      "A  -  Swing sword (hold to keep swinging)",
      "B  -  Dash through slimes",
      "Y  -  Hammer slam (smashes towers)",
      "D-pad  -  Call in care packages",
      "",
      "Start  -  Resume",
      "Back  -  Quit to level select",
  };
  const auto& lines = using_controller() ? controller_lines : keyboard_lines;

  // Line spacing follows the font
  const auto line_height =
      static_cast<float>(asw::util::get_font_height(font_medium)) + 6.0F;
  float y = 420.0F;
  for (const auto* line : lines) {
    asw::draw::text(font_medium, line, asw::Vec2f(SCREEN_W / 2.0F, y),
                    asw::Color(255, 255, 255), asw::TextJustify::Center);
    y += line_height;
  }
}
