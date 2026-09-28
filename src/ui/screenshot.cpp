#include "screenshot.h"

#include <array>
#include <asw/asw.h>
#include <ctime>
#include <string>

#include "../controls.h"

void handle_screenshot_key() {
  // A press stays down across several draws until the next update, so fire
  // once and wait for the key to be let go
  static bool armed = true;

  if (!asw::input::get_action(action::SCREENSHOT)) {
    armed = true;
    return;
  }

  if (!armed) {
    return;
  }
  armed = false;

  // No save folder in the browser
  const auto folder = asw::assets::get_save_path("adsgames", "slime-knight");
  if (folder.empty()) {
    return;
  }

  const std::time_t now = std::time(nullptr);
  std::array<char, 32> stamp{};
  std::strftime(stamp.data(), stamp.size(), "%Y%m%d-%H%M%S",
                std::localtime(&now));

  const auto path = folder + "screenshot-" + stamp.data() + ".png";
  if (asw::display::screenshot(path)) {
    asw::log::info("Saved " + path);
  } else {
    asw::log::warn("Could not save screenshot");
  }
}
