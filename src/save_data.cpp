#include "save_data.h"

#include <asw/asw.h>
#include <fstream>
#include <string>

std::array<float, LEVEL_COUNT> SaveData::best_times{};

namespace {
std::string save_file() {
  const auto folder = asw::assets::get_save_path("adsgames", "slime-knight");
  if (folder.empty()) {
    return "";
  }

  return folder + "save.dat";
}
}  // namespace

void SaveData::load() {
  best_times.fill(0.0F);

  const auto path = save_file();
  if (path.empty()) {
    return;
  }

  std::ifstream file(path);
  for (auto& time : best_times) {
    if (!(file >> time)) {
      time = 0.0F;
    }
  }
}

void SaveData::save() {
  const auto path = save_file();
  if (path.empty()) {
    return;
  }

  std::ofstream file(path);
  for (const auto time : best_times) {
    file << time << "\n";
  }
}

float SaveData::get_best_time(int level) {
  if (level < 0 || level >= LEVEL_COUNT) {
    return 0.0F;
  }

  return best_times[level];
}

bool SaveData::record_clear(int level, float time) {
  if (level < 0 || level >= LEVEL_COUNT) {
    return false;
  }

  if (best_times[level] > 0.0F && time >= best_times[level]) {
    return false;
  }

  best_times[level] = time;
  save();
  return true;
}
