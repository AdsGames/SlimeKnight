#pragma once

#include <asw/asw.h>
#include <string_view>

// Action names, bound to keyboard and controller in bind_controls()
namespace action {
// Gameplay
inline constexpr std::string_view MOVE_LEFT = "move_left";
inline constexpr std::string_view MOVE_RIGHT = "move_right";
inline constexpr std::string_view MOVE_UP = "move_up";
inline constexpr std::string_view MOVE_DOWN = "move_down";
inline constexpr std::string_view SLASH = "slash";
inline constexpr std::string_view DASH = "dash";
inline constexpr std::string_view HAMMER = "hammer";
inline constexpr std::string_view PACKAGE_1 = "package_1";
inline constexpr std::string_view PACKAGE_2 = "package_2";
inline constexpr std::string_view PACKAGE_3 = "package_3";
inline constexpr std::string_view PACKAGE_4 = "package_4";
inline constexpr std::string_view PAUSE = "pause";
inline constexpr std::string_view QUIT_LEVEL = "quit_level";
inline constexpr std::string_view RETRY = "retry";

// Menus
inline constexpr std::string_view MENU_UP = "menu_up";
inline constexpr std::string_view MENU_DOWN = "menu_down";
inline constexpr std::string_view MENU_LEFT = "menu_left";
inline constexpr std::string_view MENU_RIGHT = "menu_right";
inline constexpr std::string_view CONFIRM = "confirm";
inline constexpr std::string_view BACK = "back";

// Anywhere
inline constexpr std::string_view SCREENSHOT = "screenshot";
}  // namespace action

// Register every action, call once at start up
void bind_controls();

// Movement from keys or the left stick, length 0 to 1
asw::Vec2f read_move();

// True when the player last used a controller, for button prompts
bool using_controller();

// Pick the prompt text for the device in use
const char* prompt(const char* keyboard, const char* controller);
