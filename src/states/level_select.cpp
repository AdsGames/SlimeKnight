#include "./level_select.h"

#include <string>

#include "../save_data.h"
#include "../controls.h"
#include "../game/audio.h"
#include "../game/level.h"
#include "../ui/cursor.h"
#include "../ui/screenshot.h"


void LevelSelect::init() {
  for (int i = 0; i < LEVEL_COUNT; i++) {
    const auto n = std::to_string(i + 1);
    backgrounds[i] = asw::assets::load_texture(
        "assets/images/ui/level_select_" + n + ".png");
    thumbnails[i] = asw::assets::load_texture(
        std::string("assets/images/worlds/") + LEVELS[i].map + ".png");
  }

  background_choose =
      asw::assets::load_texture("assets/images/ui/level_select_choose.png");
  background_menu = asw::assets::load_texture("assets/images/ui/menu.png");
  frame = asw::assets::load_texture("assets/images/ui/frame.png");
  font = asw::assets::load_font("assets/fonts/ariblk.ttf", 26);
  font_small = asw::assets::load_font("assets/fonts/ariblk.ttf", 18);

  back = Button(asw::Vec2f(24, 24));
  back.set_images("assets/images/ui/button_back.png",
                  "assets/images/ui/button_back_hover.png");
  back.set_on_click([this]() {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::Menu);
  });

  selected = -1;
  Audio::play_music("menu");
}

asw::Quadf LevelSelect::thumbnail_area(int level) {
  const float center_x = 232.0F + (408.0F * static_cast<float>(level));
  return {center_x - 145.0F, 300.0F, 290.0F, 218.0F};
}

void LevelSelect::start_level(int level) {
  Audio::play(Sfx::Click);
  current_level = level;
  manager.set_next_scene(ProgramState::Game);
}

void LevelSelect::update(float dt) {
  asw::scene::Scene<ProgramState>::update(dt);

  const auto& mouse = asw::input::get_mouse();

  // Mouse hover
  if (mouse.change.magnitude() > 0) {
    int hovered = -1;
    for (int i = 0; i < LEVEL_COUNT; i++) {
      if (thumbnail_area(i).contains(mouse.position)) {
        hovered = i;
      }
    }
    if (hovered != -1 && hovered != selected) {
      Audio::play(Sfx::Click, 0.5F);
    }
    selected = hovered;
  }

  // Keyboard and controller
  if (asw::input::get_action_down(action::MENU_RIGHT)) {
    selected = (selected + 1) % LEVEL_COUNT;
    Audio::play(Sfx::Click, 0.5F);
  } else if (asw::input::get_action_down(action::MENU_LEFT)) {
    selected = selected <= 0 ? LEVEL_COUNT - 1 : selected - 1;
    Audio::play(Sfx::Click, 0.5F);
  }

  if (selected >= 0 && asw::input::get_action_down(action::CONFIRM)) {
    start_level(selected);
    return;
  }

  if (asw::input::get_action_down(action::BACK)) {
    Audio::play(Sfx::Click);
    manager.set_next_scene(ProgramState::Menu);
    return;
  }

  if (asw::input::get_mouse_button_down(asw::input::MouseButton::Left)) {
    for (int i = 0; i < LEVEL_COUNT; i++) {
      if (thumbnail_area(i).contains(mouse.position)) {
        start_level(i);
        return;
      }
    }
  }

  back.update();
}

void LevelSelect::draw() {
  // Story art is transparent at the top, the menu shows through
  asw::draw::sprite(background_menu, asw::Vec2f(0, 0));
  asw::draw::rect_fill(asw::Quadf(0, 0, SCREEN_W, SCREEN_H),
                       asw::Color(0, 0, 30, 200));
  asw::draw::sprite(selected >= 0 ? backgrounds[selected] : background_choose,
                    asw::Vec2f(0, 0));

  back.draw();

  const auto shadow_text = [](const asw::Font& f, const std::string& text,
                              const asw::Vec2f& position,
                              const asw::Color& color) {
    asw::draw::text_shadow(f, text, position, color, asw::Color(0, 0, 0),
                           asw::Vec2f(2, 2), asw::TextJustify::Center);
  };

  shadow_text(font, "Choose your battle", asw::Vec2f(SCREEN_W / 2.0F, 150),
              asw::Color(255, 220, 60));

  for (int i = 0; i < LEVEL_COUNT; i++) {
    const auto area = thumbnail_area(i);
    const float center_x = area.position.x + (area.size.x / 2.0F);

    // The frame sits behind the thumbnail as a border
    if (i == selected) {
      asw::draw::stretch_sprite(
          frame, asw::Quadf(area.position.x - 14, area.position.y - 14,
                            area.size.x + 28, area.size.y + 28));
    }

    asw::draw::stretch_sprite(thumbnails[i], area);
    asw::draw::rect(area, asw::Color(0, 0, 0));

    shadow_text(font, std::to_string(i + 1) + ". " + LEVELS[i].name,
                asw::Vec2f(center_x, 240), asw::Color(255, 255, 255));

    const float best = SaveData::get_best_time(i);
    const auto label =
        best > 0.0F ? "Best  " + format_time(best) : std::string("Not cleared");
    shadow_text(font_small, label, asw::Vec2f(center_x, 540),
                best > 0.0F ? asw::Color(120, 255, 120)
                            : asw::Color(200, 200, 200));
  }

  // Screenshots leave out the cursor
  handle_screenshot_key();

  draw_cursor();
}
