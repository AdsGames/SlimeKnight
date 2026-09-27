#pragma once

#include <array>

#include "../globals.h"

// World size in pixels, the same for every level
inline constexpr float WORLD_W = 4000.0F;
inline constexpr float WORLD_H = 3000.0F;

// Area the knight and slimes can walk in
inline constexpr float WALK_INSET = 170.0F;

// Area towers and ruins are placed in
inline constexpr float BUILD_INSET = 420.0F;

// Settings for one level
struct LevelConfig {
  const char* name;
  const char* map;
  const char* ruin;
  const char* music;
  int towers;
  int ruins;
  // Seconds between tower spawns
  float spawn_interval;
  // Chance a spawn is a red (fast) slime
  float red_chance;
  // Chance a spawn is a purple (splitting) slime
  float purple_chance;
  // Boss appears once every tower and slime is gone
  bool boss;
};

inline constexpr std::array<LevelConfig, LEVEL_COUNT> LEVELS = {{
    {"The Meadow", "scene1", "ruin_stone", "level1", 7, 41, 5.0F, 0.15F, 0.10F,
     false},
    {"The Desert", "scene2", "ruin_cactus", "level2", 10, 61, 4.4F, 0.30F,
     0.20F, false},
    {"The Keep", "scene3", "ruin_plywood", "level3", 13, 21, 3.7F, 0.35F, 0.30F,
     true},
}};
