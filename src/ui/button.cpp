#include "button.h"

#include <utility>

Button::Button(const asw::Vec2f& position) {
  transform.position = position;
}

void Button::set_on_click(std::function<void()> func) {
  on_click = std::move(func);
}

void Button::set_images(const std::string& image,
                        const std::string& hover_image) {
  images[0] = asw::assets::load_texture(image);
  images[1] = asw::assets::load_texture(hover_image);
  transform.size = asw::util::get_texture_size(images[0]);
}

void Button::set_selected(bool selected) {
  this->selected = selected;
}

bool Button::hover() const {
  return transform.contains(asw::input::get_mouse().position);
}

void Button::update() {
  if (hover() &&
      asw::input::get_mouse_button_down(asw::input::MouseButton::Left) &&
      on_click != nullptr) {
    on_click();
  }
}

void Button::draw() const {
  const auto& image = images[(hover() || selected) ? 1 : 0];
  if (image) {
    asw::draw::sprite(image, transform.position);
  } else {
    asw::draw::rect_fill(transform, asw::Color(60, 60, 60));
  }
}
