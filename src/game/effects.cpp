#include "effects.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
constexpr uint32_t PARTICLES_PER_EMITTER = 400;
constexpr std::size_t MAX_DEBRIS = 200;
constexpr std::size_t MAX_DECALS = 250;
constexpr float DECAL_LIFE = 20.0F;

// asw particles have no drag, so bursts are slowed to travel about as far
// as before
constexpr float BURST_SPEED_SCALE = 0.7F;
}  // namespace

void Effects::init(const asw::Font& small_font, const asw::Font& big_font) {
  this->small_font = small_font;
  this->big_font = big_font;
  clear();
}

void Effects::clear() {
  emitters.clear();
  debris_list.clear();
  texts.clear();
  decals.clear();
}

void Effects::update(float dt) {
  for (auto& pool : emitters) {
    pool.emitter.update(dt);
  }

  for (auto& d : debris_list) {
    d.life -= dt;
    d.velocity.y += 1400.0F * dt;
    d.position += d.velocity * dt;
    d.angle += d.spin * dt;

    // Bounce off the ground, losing most of the energy each time
    if (d.position.y > d.ground && d.velocity.y > 0.0F) {
      d.position.y = d.ground;
      d.velocity.y *= -0.35F;
      d.velocity.x *= 0.6F;
      d.spin *= 0.6F;
    }
  }
  std::erase_if(debris_list, [](const Debris& d) { return d.life <= 0.0F; });

  for (auto& t : texts) {
    t.life -= dt;
    t.position.y -= 45.0F * dt;
  }
  std::erase_if(texts, [](const FloatingText& t) { return t.life <= 0.0F; });

  for (auto& d : decals) {
    d.life -= dt;
  }
  std::erase_if(decals, [](const Decal& d) { return d.life <= 0.0F; });
}

void Effects::draw_ground(const asw::Camera& camera) const {
  const auto view = camera.get_view();

  for (const auto& d : decals) {
    if (!view.collides(d.transform)) {
      continue;
    }

    const float alpha = std::clamp(d.life / 3.0F, 0.0F, 1.0F) * 0.85F;
    asw::draw::set_alpha(d.texture, alpha);
    asw::draw::stretch_sprite(d.texture, camera.world_to_screen(d.transform));
    asw::draw::set_alpha(d.texture, 1.0F);
  }
}

void Effects::draw_top(const asw::Camera& camera) const {
  const auto view = camera.get_view();

  for (const auto& d : debris_list) {
    if (!view.contains(d.position)) {
      continue;
    }

    auto color = d.color;
    color.a = static_cast<uint8_t>(
        255.0F * std::clamp(d.life / (d.max_life * 0.3F), 0.0F, 1.0F));
    const auto screen = camera.world_to_screen(d.position);
    asw::draw::rect_fill_rotate(
        asw::Quadf(screen.x - d.size.x / 2, screen.y - d.size.y / 2, d.size.x,
                   d.size.y),
        d.angle, color);
  }

  for (auto& pool : emitters) {
    pool.emitter.draw(camera);
  }

  for (const auto& t : texts) {
    const float fade = std::clamp(t.life / (t.max_life * 0.5F), 0.0F, 1.0F);
    const auto screen = camera.world_to_screen(t.position);
    const auto& font = t.big ? big_font : small_font;

    auto color = t.color;
    color.a = static_cast<uint8_t>(255.0F * fade);
    asw::draw::text_shadow(font, t.text, screen, color, asw::Color(0, 0, 0, 200),
                           asw::Vec2f(2, 2), asw::TextJustify::Center);
  }
}

asw::ParticleEmitter& Effects::emitter_for(const asw::Color& color,
                                           float speed,
                                           float size,
                                           float gravity,
                                           bool ring) {
  for (auto& pool : emitters) {
    if (pool.color.r == color.r && pool.color.g == color.g &&
        pool.color.b == color.b && pool.color.a == color.a &&
        pool.speed == speed && pool.size == size && pool.gravity == gravity &&
        pool.ring == ring) {
      return pool.emitter;
    }
  }

  asw::ParticleConfig config;
  config.color_start = color;
  config.color_end = color;
  config.alpha_start = static_cast<float>(color.a) / 255.0F;
  config.alpha_end = 0.0F;
  config.gravity = asw::Vec2f(0.0F, gravity);

  if (ring) {
    // Every particle leaves at the same speed, so they stay in a ring
    config.speed_min = speed;
    config.speed_max = speed;
    config.lifetime_min = 0.45F;
    config.lifetime_max = 0.45F;
    config.size_start = size;
    config.size_end = size * 0.4F;
  } else {
    config.speed_min = speed * 0.3F * BURST_SPEED_SCALE;
    config.speed_max = speed * BURST_SPEED_SCALE;
    config.lifetime_min = 0.35F;
    config.lifetime_max = 0.8F;
    config.size_start = size * 1.3F;
    config.size_end = size * 0.3F;
  }

  // Alpha is carried by alpha_start, the colour itself stays opaque
  config.color_start.a = 255;
  config.color_end.a = 255;

  emitters.push_back(EmitterPool{color, speed, size, gravity, ring,
                                 asw::ParticleEmitter(config,
                                                      PARTICLES_PER_EMITTER)});
  return emitters.back().emitter;
}

void Effects::burst(const asw::Vec2f& position,
                    const asw::Color& color,
                    int count,
                    float speed,
                    float size,
                    float gravity) {
  auto& emitter = emitter_for(color, speed, size, gravity, false);
  emitter.transform.position = position;
  emitter.emit(static_cast<uint32_t>(count));
}

void Effects::ring(const asw::Vec2f& position,
                   float radius,
                   const asw::Color& color) {
  // Travels about the radius over the particle lifetime
  auto& emitter = emitter_for(color, radius / 0.45F, 10.0F, 0.0F, true);
  emitter.transform.position = position;
  emitter.emit(48);
}

void Effects::debris(const asw::Vec2f& position,
                     float ground,
                     const asw::Color& color,
                     int count) {
  for (int i = 0; i < count && debris_list.size() < MAX_DEBRIS; i++) {
    Debris d;
    d.position = position + asw::Vec2f(asw::random::between(-50.0F, 50.0F),
                                       asw::random::between(-60.0F, 30.0F));
    d.velocity = asw::Vec2f(asw::random::between(-380.0F, 380.0F),
                            asw::random::between(-760.0F, -260.0F));
    d.size = asw::Vec2f(asw::random::between(8.0F, 22.0F),
                        asw::random::between(6.0F, 16.0F));

    // Vary the shade so the pile does not look flat
    const float shade = asw::random::between(0.7F, 1.2F);
    d.color = asw::Color(
        static_cast<uint8_t>(std::min(255.0F, color.r * shade)),
        static_cast<uint8_t>(std::min(255.0F, color.g * shade)),
        static_cast<uint8_t>(std::min(255.0F, color.b * shade)));

    d.angle = asw::random::between(0.0F, 2.0F * std::numbers::pi_v<float>);
    d.spin = asw::random::between(-14.0F, 14.0F);
    d.ground = ground + asw::random::between(-20.0F, 70.0F);
    d.max_life = asw::random::between(1.6F, 2.6F);
    d.life = d.max_life;
    debris_list.push_back(d);
  }
}

void Effects::text(const asw::Vec2f& position,
                   const std::string& text,
                   const asw::Color& color,
                   bool big) {
  FloatingText t;
  t.position = position;
  t.text = text;
  t.color = color;
  t.max_life = big ? 1.4F : 0.9F;
  t.life = t.max_life;
  t.big = big;
  texts.push_back(t);
}

void Effects::decal(const asw::Texture& texture, const asw::Quadf& transform) {
  if (decals.size() >= MAX_DECALS) {
    decals.erase(decals.begin());
  }

  decals.push_back(Decal{texture, transform, DECAL_LIFE});
}
