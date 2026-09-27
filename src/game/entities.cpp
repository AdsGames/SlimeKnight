#include "entities.h"

int Slime::damage() const {
  switch (type) {
    case SlimeType::Green:
      return 8;
    case SlimeType::Red:
      return 10;
    case SlimeType::Purple:
      return 12;
    case SlimeType::Mini:
      return 5;
    case SlimeType::Boss:
      return 20;
  }

  return 8;
}

asw::Vec2f Slime::draw_size() const {
  // Sprite frames are 72x46, keep that ratio
  const float width = radius * 2.5F;
  return {width, width * (46.0F / 72.0F)};
}

asw::Quadf Slime::hitbox() const {
  const auto size = draw_size();
  return {position.x - (size.x * 0.4F), position.y - (size.y * 0.75F),
          size.x * 0.8F, size.y * 0.75F};
}

asw::Quadf Tower::footprint() const {
  return {base.x - 60.0F, base.y - 70.0F, 120.0F, 70.0F};
}

asw::Quadf Tower::sprite_area() const {
  return {base.x - WIDTH / 2.0F, base.y - HEIGHT, WIDTH, HEIGHT};
}

asw::Quadf Package::box() const {
  return {target.x - BOX_SIZE / 2.0F, target.y - BOX_SIZE, BOX_SIZE, BOX_SIZE};
}
