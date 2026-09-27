/**
 * Scene ids for the scene manager
 * Each scene only handles its own logic, drawing and transitions
 */

#pragma once

enum class ProgramState {
  Null,
  Init,
  Splash,
  Menu,
  LevelSelect,
  Game,
  Results,
  Die,
  Exit,
};
