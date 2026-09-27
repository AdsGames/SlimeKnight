#pragma once

#include <asw/asw.h>
#include <string>

// Sound effects used in game
enum class Sfx {
  Click,
  Drop,
  GateClose,
  GateOpen,
  Step,
  Hurt,
  SlimeHit,
  SlimeSpawn1,
  SlimeSpawn2,
  Splat,
  Sword,
  TowerDestroy,
  Wind,
  Count,
};

// Loads every sound once and plays them with positional panning
class Audio {
 public:
  static void load();

  // Play a sound
  static void play(Sfx sfx, float volume = 1.0F);

  // Play a sound panned by its screen x position
  static void play_at(Sfx sfx, float screen_x, float volume = 1.0F);

  // Switch the music track, does nothing if it is already playing
  static void play_music(const std::string& name);

 private:
  static std::string current_music;
};
