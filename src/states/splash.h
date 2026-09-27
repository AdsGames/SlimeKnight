#pragma once

#include <asw/asw.h>

#include "./state.h"

// Title card, any key continues
class Splash : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  asw::Texture background;
  float timer{0.0F};
};
