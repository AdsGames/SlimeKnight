#pragma once

#include <asw/asw.h>

#include "./state.h"

// Shown when the knight falls
class Die : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;

 private:
  asw::Texture background;
  asw::Font font;
  float timer{0.0F};
};
