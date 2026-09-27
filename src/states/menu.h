#pragma once

#include <asw/asw.h>
#include <array>

#include "../ui/button.h"
#include "./state.h"

// Main menu with start, help and quit
class Menu : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  void draw_help() const;

  enum ButtonName { BUTTON_START, BUTTON_HELP, BUTTON_QUIT, NUM_BUTTONS };

  std::array<Button, NUM_BUTTONS> buttons;

  asw::Texture background;
  asw::Texture help_background;
  asw::Font font_title;
  asw::Font font_text;

  int selected{-1};
  bool show_help{false};
};
