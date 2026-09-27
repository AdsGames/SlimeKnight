#include "./splash.h"

#include <algorithm>

#include "../game/audio.h"
#include "../globals.h"

void Splash::init() {
  background = asw::assets::load_texture("assets/images/ui/splash.png");
  timer = 0.0F;
  Audio::play_music("menu");
}

void Splash::update(float dt) {
  timer += dt;

  // Short delay so a held key does not skip straight past
  if (timer < 0.4F) {
    return;
  }

  const bool controller =
      asw::input::get_controller_count() > 0 &&
      (asw::input::get_controller_button_down(
           0, asw::input::ControllerButton::A) ||
       asw::input::get_controller_button_down(
           0, asw::input::ControllerButton::Start));

  if (asw::input::get_keyboard().any_pressed ||
      asw::input::get_mouse_button_down(asw::input::MouseButton::Left) ||
      controller) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::Menu);
  }
}

void Splash::draw() {
  asw::draw::sprite(background, asw::Vec2f(0, 0));

  // Fade in from black
  const float fade = std::clamp(1.0F - (timer / 0.6F), 0.0F, 1.0F);
  if (fade > 0.0F) {
    asw::draw::rect_fill(asw::Quadf(0, 0, SCREEN_W, SCREEN_H),
                         asw::Color(0, 0, 0, static_cast<uint8_t>(255 * fade)));
  }
}
