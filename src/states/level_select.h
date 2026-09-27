#pragma once

#include <asw/asw.h>
#include <array>

#include "../globals.h"
#include "../ui/button.h"
#include "./state.h"

// Pick one of the three levels
class LevelSelect : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  void start_level(int level);
  static asw::Quadf thumbnail_area(int level);

  std::array<asw::Texture, LEVEL_COUNT> backgrounds;
  std::array<asw::Texture, LEVEL_COUNT> thumbnails;
  asw::Texture background_choose;
  asw::Texture background_menu;
  asw::Texture frame;
  asw::Font font;
  asw::Font font_small;
  Button back;

  // Level under the mouse or picked with the keyboard, -1 for none
  int selected{-1};
};
