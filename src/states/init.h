#pragma once

#include <asw/asw.h>

#include "./state.h"

// Loads shared resources then moves to the splash screen
class Init : public asw::scene::Scene<ProgramState> {
 public:
  using asw::scene::Scene<ProgramState>::Scene;

  void init() override;
  void update(float dt) override;
  void draw() override;
};
