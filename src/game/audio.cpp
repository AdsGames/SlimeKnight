#include "audio.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <unordered_map>

std::string Audio::current_music;

namespace {
using asw::sound::Bus;

// How each sound is played
struct SfxInfo {
  const char* file;
  Bus bus;
  // Random pitch and volume change per play, so repeats do not sound robotic
  float pitch_variation;
  float volume_variation;
  // Busy voices go to higher priority sounds first
  int priority;
};

constexpr std::array<SfxInfo, static_cast<std::size_t>(Sfx::Count)> SFX = {{
    {"click", Bus::Ui, 0.0F, 0.0F, 3},
    {"drop", Bus::Sfx, 0.05F, 0.1F, 1},
    {"gate_close", Bus::Ambient, 0.08F, 0.1F, 0},
    {"gate_open", Bus::Ambient, 0.08F, 0.1F, 0},
    {"grass", Bus::Sfx, 0.15F, 0.25F, 0},
    {"hurt", Bus::Sfx, 0.08F, 0.1F, 2},
    {"slime_kill", Bus::Sfx, 0.12F, 0.15F, 1},
    {"slime1", Bus::Sfx, 0.12F, 0.15F, 0},
    {"slime2", Bus::Sfx, 0.12F, 0.15F, 0},
    {"splat", Bus::Sfx, 0.15F, 0.15F, 1},
    {"sword", Bus::Sfx, 0.1F, 0.1F, 1},
    {"tower_destroy", Bus::Sfx, 0.05F, 0.0F, 3},
    {"wind", Bus::Ambient, 0.05F, 0.0F, 1},
}};

// Full volume within about half a screen, silent past about a screen
// and a half, so off screen fights are faint rather than gone
const asw::sound::Attenuation WORLD_ATTENUATION{
    500.0F, 2000.0F, asw::sound::Rolloff::Linear};

std::array<asw::Sample, static_cast<std::size_t>(Sfx::Count)> samples;

// Music is loaded the first time it is played and kept around
std::unordered_map<std::string, asw::Music> music_cache;

asw::sound::PlayOptions options_for(Sfx sfx, float volume) {
  const auto& info = SFX[static_cast<std::size_t>(sfx)];

  asw::sound::PlayOptions options;
  options.volume = std::clamp(volume, 0.0F, 1.0F);
  options.bus = info.bus;
  options.pitch_variation = info.pitch_variation;
  options.volume_variation = info.volume_variation;
  options.priority = info.priority;
  options.attenuation = WORLD_ATTENUATION;
  return options;
}
}  // namespace

void Audio::load() {
  for (std::size_t i = 0; i < SFX.size(); i++) {
    samples[i] = asw::assets::load_sample(std::string("assets/sounds/") +
                                          SFX[i].file + ".wav");
  }
}

asw::sound::SoundHandle Audio::play(Sfx sfx, float volume) {
  const auto& sample = samples[static_cast<std::size_t>(sfx)];
  if (!sample) {
    return {};
  }

  return asw::sound::play(sample, options_for(sfx, volume));
}

asw::sound::SoundHandle Audio::play_at(Sfx sfx,
                                       const asw::Vec2f& position,
                                       float volume) {
  const auto& sample = samples[static_cast<std::size_t>(sfx)];
  if (!sample) {
    return {};
  }

  return asw::sound::play_positional(sample, position,
                                     options_for(sfx, volume));
}

void Audio::set_listener(const asw::Vec2f& position) {
  asw::sound::set_listener(position);
}

void Audio::duck_music(float gain, float hold_s) {
  asw::sound::duck(Bus::Music, gain, hold_s, 0.15F);
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
