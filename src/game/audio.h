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

// Loads every sound once and plays each with its own mix settings
class Audio {
 public:
  static void load();

  // Play a sound that is not placed in the world, such as the knight's own
  // sounds or menu clicks
  static asw::sound::SoundHandle play(Sfx sfx, float volume = 1.0F);

  // Play a sound at a point in the world, heard from the listener
  static asw::sound::SoundHandle play_at(Sfx sfx,
                                         const asw::Vec2f& position,
                                         float volume = 1.0F);

  // Where positional sounds are heard from, usually the knight
  static void set_listener(const asw::Vec2f& position);

  // Turn the music down for a moment so a big event stands out
  static void duck_music(float gain, float hold_s);

  // Switch the music track, does nothing if it is already playing
  static void play_music(const std::string& name);

 private:
  static std::string current_music;
};
