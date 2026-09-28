#include "controls.h"

#include <initializer_list>

namespace {
using asw::input::ANY_CONTROLLER;
using asw::input::ControllerAxis;
using asw::input::ControllerAxisBinding;
using asw::input::ControllerButton;
using asw::input::ControllerButtonBinding;
using asw::input::Key;
using asw::input::KeyBinding;

void keys(std::string_view name, std::initializer_list<Key> list) {
  for (const auto key : list) {
    asw::input::bind_action(name, KeyBinding{key});
  }
}

void buttons(std::string_view name, std::initializer_list<ControllerButton> list) {
  for (const auto button : list) {
    asw::input::bind_action(name,
                            ControllerButtonBinding{button, ANY_CONTROLLER});
  }
}

void stick(std::string_view name, ControllerAxis axis, bool positive) {
  asw::input::bind_action(
      name, ControllerAxisBinding{axis, ANY_CONTROLLER, 0.5F, positive});
}
}  // namespace

void bind_controls() {
  asw::input::clear_actions();

  // Movement keys, the stick is read directly in read_move()
  keys(action::MOVE_LEFT, {Key::A, Key::Left});
  keys(action::MOVE_RIGHT, {Key::D, Key::Right});
  keys(action::MOVE_UP, {Key::W, Key::Up});
  keys(action::MOVE_DOWN, {Key::S, Key::Down});

  keys(action::SLASH, {Key::Space, Key::J});
  buttons(action::SLASH, {ControllerButton::A});

  keys(action::DASH, {Key::LShift, Key::RShift, Key::K});
  buttons(action::DASH, {ControllerButton::B});

  keys(action::HAMMER, {Key::E, Key::L});
  buttons(action::HAMMER, {ControllerButton::Y});

  keys(action::PACKAGE_1, {Key::Num1});
  buttons(action::PACKAGE_1, {ControllerButton::DPadUp});
  keys(action::PACKAGE_2, {Key::Num2});
  buttons(action::PACKAGE_2, {ControllerButton::DPadLeft});
  keys(action::PACKAGE_3, {Key::Num3});
  buttons(action::PACKAGE_3, {ControllerButton::DPadRight});
  keys(action::PACKAGE_4, {Key::Num4});
  buttons(action::PACKAGE_4, {ControllerButton::DPadDown});

  keys(action::PAUSE, {Key::Escape, Key::P});
  buttons(action::PAUSE, {ControllerButton::Start});

  keys(action::QUIT_LEVEL, {Key::Q, Key::Backspace});
  buttons(action::QUIT_LEVEL, {ControllerButton::Back});

  keys(action::RETRY, {Key::R});
  buttons(action::RETRY, {ControllerButton::Y});

  // Menus take the stick as well as the d-pad
  keys(action::MENU_UP, {Key::W, Key::Up});
  buttons(action::MENU_UP, {ControllerButton::DPadUp});
  stick(action::MENU_UP, ControllerAxis::LeftY, false);

  keys(action::MENU_DOWN, {Key::S, Key::Down});
  buttons(action::MENU_DOWN, {ControllerButton::DPadDown});
  stick(action::MENU_DOWN, ControllerAxis::LeftY, true);

  keys(action::MENU_LEFT, {Key::A, Key::Left});
  buttons(action::MENU_LEFT, {ControllerButton::DPadLeft});
  stick(action::MENU_LEFT, ControllerAxis::LeftX, false);

  keys(action::MENU_RIGHT, {Key::D, Key::Right});
  buttons(action::MENU_RIGHT, {ControllerButton::DPadRight});
  stick(action::MENU_RIGHT, ControllerAxis::LeftX, true);

  keys(action::CONFIRM, {Key::Return, Key::Space});
  buttons(action::CONFIRM, {ControllerButton::A, ControllerButton::Start});

  keys(action::BACK, {Key::Escape, Key::Backspace});
  buttons(action::BACK, {ControllerButton::B});

  keys(action::SCREENSHOT, {Key::F12});
}

asw::Vec2f read_move() {
  asw::Vec2f move(0, 0);

  if (asw::input::get_action(action::MOVE_LEFT)) {
    move.x -= 1.0F;
  }
  if (asw::input::get_action(action::MOVE_RIGHT)) {
    move.x += 1.0F;
  }
  if (asw::input::get_action(action::MOVE_UP)) {
    move.y -= 1.0F;
  }
  if (asw::input::get_action(action::MOVE_DOWN)) {
    move.y += 1.0F;
  }

  // Stick has its dead zone applied already, and keeps analogue speed
  const auto stick_move = asw::input::get_controller_stick(
      ANY_CONTROLLER, asw::input::ControllerStick::Left);
  if (stick_move.magnitude() > 0.0F) {
    move = stick_move;
  }

  // Diagonals are not faster
  if (move.magnitude() > 1.0F) {
    move = move.normalized();
  }

  return move;
}

bool using_controller() {
  return asw::input::get_last_device() ==
         asw::input::InputDevice::Controller;
}

const char* prompt(const char* keyboard, const char* controller) {
  return using_controller() ? controller : keyboard;
}
