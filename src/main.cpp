#include <asw/asw.h>

#include "./states/die.h"
#include "./states/game.h"
#include "./states/init.h"
#include "./states/level_select.h"
#include "./states/menu.h"
#include "./states/results.h"
#include "./states/splash.h"
#include "./states/state.h"
#include "globals.h"

int main() {
  asw::core::init(SCREEN_W, SCREEN_H);

  auto app = asw::scene::SceneManager<ProgramState>();
  app.register_scene<Init>(ProgramState::Init, app);
  app.register_scene<Splash>(ProgramState::Splash, app);
  app.register_scene<Menu>(ProgramState::Menu, app);
  app.register_scene<LevelSelect>(ProgramState::LevelSelect, app);
  app.register_scene<Game>(ProgramState::Game, app);
  app.register_scene<Results>(ProgramState::Results, app);
  app.register_scene<Die>(ProgramState::Die, app);
  app.set_next_scene(ProgramState::Init);

  app.start();

  return 0;
}
