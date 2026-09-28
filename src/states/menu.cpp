#include "./menu.h"

#include "../controls.h"
#include "../game/audio.h"
#include "../globals.h"
#include "../ui/cursor.h"
#include "../ui/screenshot.h"


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
  buttons[BUTTON_QUIT].set_on_click([]() { quit(); });

  selected = -1;
  show_help = false;
  Audio::play_music("menu");
}

void Menu::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);

  if (show_help) {
    if (asw::input::get_mouse_button_down(asw::input::MouseButton::Left) ||
        asw::input::get_keyboard().any_pressed ||
        asw::input::get_action_down(action::CONFIRM) ||
        asw::input::get_action_down(action::BACK)) {
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

  if (asw::input::get_action_down(action::MENU_DOWN)) {
    selected = (selected + 1) % NUM_BUTTONS;
    Audio::play(Sfx::Click, 0.5F);
  } else if (asw::input::get_action_down(action::MENU_UP)) {
    selected = (selected + NUM_BUTTONS - 1) % NUM_BUTTONS;
    Audio::play(Sfx::Click, 0.5F);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].set_selected(i == selected);
  }

  const bool confirm = asw::input::get_action_down(action::CONFIRM);

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
    quit();
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

  // Screenshots leave out the cursor
  handle_screenshot_key();

  draw_cursor();
}

void Menu::quit() {
  Audio::play(Sfx::Click);

  // Native dialogs only on desktop, closing the tab is the way out on the web
#ifndef __EMSCRIPTEN__
  if (!asw::dialog::confirm("Slime Knight", "Quit the game?")) {
    return;
  }
#endif

  asw::core::exit();
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

  // Line spacing follows the font
  const auto line_height =
      static_cast<float>(asw::util::get_font_height(font_text)) + 5.0F;
  float y = 140.0F;
  for (const auto* line : lines) {
    asw::draw::text(font_text, line, asw::Vec2f(60, y), asw::Color(0, 0, 0));
    y += line_height;
  }

  asw::draw::text(font_text, "Click or press any key to go back",
                  asw::Vec2f(SCREEN_W - 60.0F, SCREEN_H - 60.0F),
                  asw::Color(255, 255, 255), asw::TextJustify::Right);
}
