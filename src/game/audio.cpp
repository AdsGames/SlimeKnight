#include "audio.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <unordered_map>

#include "../globals.h"

std::string Audio::current_music;

namespace {
constexpr std::array<const char*, static_cast<std::size_t>(Sfx::Count)>
    SFX_FILES = {
        "click", "drop",          "gate_close", "gate_open", "grass",
        "hurt",  "slime_kill",    "slime1",     "slime2",    "splat",
        "sword", "tower_destroy", "wind",
};

std::array<asw::Sample, static_cast<std::size_t>(Sfx::Count)> samples;

// Music is loaded the first time it is played and kept around
std::unordered_map<std::string, asw::Music> music_cache;
}  // namespace

void Audio::load() {
  for (std::size_t i = 0; i < SFX_FILES.size(); i++) {
    samples[i] = asw::assets::load_sample(std::string("assets/sounds/") +
                                          SFX_FILES[i] + ".wav");
  }
}

void Audio::play(Sfx sfx, float volume) {
  const auto& sample = samples[static_cast<std::size_t>(sfx)];
  if (sample) {
    asw::sound::play(sample, std::clamp(volume, 0.0F, 1.0F));
  }
}

void Audio::play_at(Sfx sfx, float screen_x, float volume) {
  const auto& sample = samples[static_cast<std::size_t>(sfx)];
  if (!sample) {
    return;
  }

  const float center = SCREEN_W / 2.0F;
  const float offset = (screen_x - center) / center;

  // Fade out sounds far off screen
  const float falloff = std::clamp(2.0F - std::abs(offset), 0.0F, 1.0F);
  if (falloff <= 0.0F) {
    return;
  }

  asw::sound::play(sample, std::clamp(volume * falloff, 0.0F, 1.0F),
                   std::clamp(offset * 0.7F, -1.0F, 1.0F));
}

void Audio::play_music(const std::string& name) {
  if (name == current_music && asw::sound::is_music_playing()) {
    return;
  }

  auto& music = music_cache[name];
  if (!music) {
    music = asw::assets::load_music("assets/music/" + name + ".ogg");
  }

  current_music = name;
  asw::sound::play_music(music, 0.8F);
}
