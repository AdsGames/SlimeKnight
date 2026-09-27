#include "cursor.h"

#include <asw/asw.h>

void draw_cursor() {
  static const auto texture =
      asw::assets::load_texture("assets/images/ui/cursor.png");

  const auto& mouse = asw::input::get_mouse().position;
  asw::draw::stretch_sprite(texture, asw::Quadf(mouse.x, mouse.y, 32, 32));
}
