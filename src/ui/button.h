#pragma once

#include <asw/asw.h>
#include <array>
#include <functional>
#include <string>

// Image button with a hover state
class Button {
 public:
  Button() = default;
  explicit Button(const asw::Vec2f& position);

  void update();
  void draw() const;

  void set_images(const std::string& image, const std::string& hover_image);
  void set_on_click(std::function<void()> func);

  // Force the hover image, used for keyboard selection
  void set_selected(bool selected);

  bool hover() const;
  const asw::Quadf& get_transform() const { return transform; }

 private:
  std::function<void()> on_click;
  asw::Quadf transform{0, 0, 0, 0};
  std::array<asw::Texture, 2> images;
  bool selected{false};
};
