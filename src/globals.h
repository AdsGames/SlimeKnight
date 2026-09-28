#pragma once

#include <array>
#include <string>

// Logical screen size
inline constexpr int SCREEN_W = 1280;
inline constexpr int SCREEN_H = 960;

inline constexpr int LEVEL_COUNT = 3;

// Stats for the level that was last played
struct LevelStats {
  int kills{0};
  int damage_taken{0};
  int power_received{0};
  int energy_used{0};
  int best_combo{0};
  int towers{0};
  float time{0.0F};
  bool new_best{false};
};

// Level being played, 0 based
extern int current_level;

// Stats of the last level played, shown on the results and death screens
extern LevelStats last_stats;

// Format seconds as m:ss
std::string format_time(float seconds);
