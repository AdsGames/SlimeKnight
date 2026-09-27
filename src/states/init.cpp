#include "./init.h"

#include "../save_data.h"
#include "../game/audio.h"

void Init::init() {
  asw::display::set_title("Setting up");
  asw::display::set_icon("assets/icon.ico");

  // Primitives use alpha for overlays and particles
  asw::display::set_blend_mode(asw::BlendMode::Blend);

  // Menus draw their own sword cursor
  SDL_HideCursor();

  Audio::load();
  SaveData::load();

  asw::display::set_title("Slime Knight");
}

void Init::update(float /*dt*/) {
  manager.set_next_scene(ProgramState::Splash);
}

void Init::draw() {
  asw::draw::clear_color(asw::Color(0, 0, 0));
}
