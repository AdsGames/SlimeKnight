#pragma once

#include <array>

#include "globals.h"

// Best clear times per level, kept between sessions
class SaveData {
 public:
  static void load();
  static void save();

  // Best time in seconds, 0 when the level has not been cleared
  static float get_best_time(int level);

  // Record a clear, returns true when it beats the best time
  static bool record_clear(int level, float time);

 private:
  static std::array<float, LEVEL_COUNT> best_times;
};
