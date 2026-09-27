#include "globals.h"

#include <cstdio>

int current_level = 0;
LevelStats last_stats;

std::string format_time(float seconds) {
  const int total = static_cast<int>(seconds);
  std::array<char, 16> buffer{};
  std::snprintf(buffer.data(), buffer.size(), "%d:%02d", total / 60,
                total % 60);
  return {buffer.data()};
}
