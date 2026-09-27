#include "./menu.h"

#include "../game/audio.h"
#include "../globals.h"
#include "../ui/cursor.h"

namespace {
using asw::input::ControllerButton;
using asw::input::Key;

bool controller_down(ControllerButton button) {
  return asw::input::get_controller_count() > 0 &&
         asw::input::get_controller_button_down(0, button);
}
}  // namespace

void Menu::init() {
  background = asw::assets::load_texture("assets/images/ui/menu.png");
  help_background = asw::assets::load_texture("assets/images/ui/help_bg.png");
  font_title = asw::assets::load_font("assets/fonts/ariblk.ttf", 48);
  font_text = asw::assets::load_font("assets/fonts/ariblk.ttf", 22);

  buttons[BUTTON_START] = Button(asw::Vec2f(510, 300));
  buttons[BUTTON_HELP] = Button(asw::Vec2f(510, 400));
  buttons[BUTTON_QUIT] = Button(asw::Vec2f(510, 500));

  buttons[BUTTON_START].set_images("assets/images/ui/button_start.png",
                                   "assets/images/ui/button_start_hover.png");
  buttons[BUTTON_HELP].set_images("assets/images/ui/button_help.png",
                                  "assets/images/ui/button_help_hover.png");
  buttons[BUTTON_QUIT].set_images("assets/images/ui/button_quit.png",
                                  "assets/images/ui/button_quit_hover.png");

  buttons[BUTTON_START].set_on_click([this]() {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::LevelSelect);
  });
  buttons[BUTTON_HELP].set_on_click([this]() {
    Audio::play(Sfx::Click);
    show_help = true;
  });
  buttons[BUTTON_QUIT].set_on_click([]() { asw::core::exit(); });

  selected = -1;
  show_help = false;
  Audio::play_music("menu");
}

void Menu::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);

  if (show_help) {
    if (asw::input::get_mouse_button_down(asw::input::MouseButton::Left) ||
        asw::input::get_keyboard().any_pressed ||
        controller_down(ControllerButton::A) ||
        controller_down(ControllerButton::B)) {
      Audio::play(Sfx::Click);
      show_help = false;
    }
    return;
  }

  // Hover and keyboard selection share one highlight
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (buttons[i].hover() && asw::input::get_mouse().change.magnitude() > 0 &&
        selected != i) {
      selected = i;
      Audio::play(Sfx::Click, 0.5F);
    }
  }

  if (asw::input::get_key_down(Key::Down) ||
      asw::input::get_key_down(Key::S) ||
      controller_down(ControllerButton::DPadDown)) {
    selected = (selected + 1) % NUM_BUTTONS;
    Audio::play(Sfx::Click, 0.5F);
  } else if (asw::input::get_key_down(Key::Up) ||
             asw::input::get_key_down(Key::W) ||
             controller_down(ControllerButton::DPadUp)) {
    selected = (selected + NUM_BUTTONS - 1) % NUM_BUTTONS;
    Audio::play(Sfx::Click, 0.5F);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].set_selected(i == selected);
  }

  const bool confirm = asw::input::get_key_down(Key::Return) ||
                       asw::input::get_key_down(Key::Space) ||
                       controller_down(ControllerButton::A);

  if (confirm && selected == BUTTON_START) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::LevelSelect);
    return;
  }
  if (confirm && selected == BUTTON_HELP) {
    Audio::play(Sfx::Click);
    show_help = true;
    return;
  }
  if (confirm && selected == BUTTON_QUIT) {
    asw::core::exit();
    return;
  }

  for (auto& button : buttons) {
    button.update();
  }
}

void Menu::draw() {
  asw::draw::sprite(background, asw::Vec2f(0, 0));

  for (const auto& button : buttons) {
    button.draw();
  }

  if (show_help) {
    draw_help();
  }

  draw_cursor();
}

void Menu::draw_help() const {
  asw::draw::stretch_sprite(help_background,
                            asw::Quadf(0, 0, SCREEN_W, SCREEN_H));
  asw::draw::text(font_title, "Help", asw::Vec2f(60, 50), asw::Color(0, 0, 0));

  const std::array<const char*, 20> lines = {
      "Slime towers have overrun the land. Smash every tower",
      "and slay every remaining slime to free it.",
      "",
      "WASD / Arrows - Move",
      "Space / J - Swing your sword. Hold it to keep swinging.",
      "Shift / K - Dash. You can not be hurt while dashing.",
      "E / L - Hammer slam. Crumbles nearby towers and flattens slimes.",
      "1 - 4 - Call in a care package: energy regen, health,",
      "            energy or speed. It lands in front of you.",
      "Esc / P - Pause",
      "",
      "Swinging, dashing and slamming use energy, which refills over time.",
      "Slaying slimes gives power. Chain kills for a combo and more power.",
      "The hammer costs 30 power and 30 energy. Watch for the knight",
      "icon on the top bar to know when it is ready.",
      "",
      "Green slimes are slow, red slimes are fast and purple slimes",
      "split in two. Towers spawn faster as their numbers fall...",
      "",
      "Gamepads work too: A swing, B dash, Y hammer, D-pad packages.",
  };

  float y = 140.0F;
  for (const auto* line : lines) {
    asw::draw::text(font_text, line, asw::Vec2f(60, y), asw::Color(0, 0, 0));
    y += 36.0F;
  }

  asw::draw::text(font_text, "Click or press any key to go back",
                  asw::Vec2f(SCREEN_W - 60.0F, SCREEN_H - 60.0F),
                  asw::Color(255, 255, 255), asw::TextJustify::Right);
}
