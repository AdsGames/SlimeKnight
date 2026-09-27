#include "effects.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
constexpr std::size_t MAX_PARTICLES = 1500;
constexpr std::size_t MAX_DECALS = 250;
constexpr float DECAL_LIFE = 20.0F;
}  // namespace

void Effects::init(const asw::Font& small_font, const asw::Font& big_font) {
  this->small_font = small_font;
  this->big_font = big_font;
  clear();
}

void Effects::clear() {
  particles.clear();
  texts.clear();
  decals.clear();
}

void Effects::update(float dt) {
  for (auto& p : particles) {
    p.life -= dt;
    p.velocity.y += p.gravity * dt;
    p.velocity = p.velocity * std::exp(-2.5F * dt);
    p.position += p.velocity * dt;
  }
  std::erase_if(particles, [](const Particle& p) { return p.life <= 0.0F; });

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

void Effects::draw_ground(const Camera& camera) const {
  const auto view = camera.get_view();

  for (const auto& d : decals) {
    if (!view.collides(d.transform)) {
      continue;
    }

    const float alpha = std::clamp(d.life / 3.0F, 0.0F, 1.0F) * 0.85F;
    asw::draw::set_alpha(d.texture, alpha);
    asw::draw::stretch_sprite(d.texture, camera.to_screen(d.transform));
    asw::draw::set_alpha(d.texture, 1.0F);
  }
}

void Effects::draw_top(const Camera& camera) const {
  const auto view = camera.get_view();

  for (const auto& p : particles) {
    if (!view.contains(p.position)) {
      continue;
    }

    const float t = std::clamp(p.life / p.max_life, 0.0F, 1.0F);
    const float size = std::max(1.0F, p.size * t);
    auto color = p.color;
    color.a = static_cast<uint8_t>(static_cast<float>(color.a) * t);
    const auto screen = camera.to_screen(p.position);
    asw::draw::rect_fill(
        asw::Quadf(screen.x - size / 2, screen.y - size / 2, size, size),
        color);
  }

  for (const auto& t : texts) {
    const float fade = std::clamp(t.life / (t.max_life * 0.5F), 0.0F, 1.0F);
    const auto screen = camera.to_screen(t.position);
    const auto& font = t.big ? big_font : small_font;

    auto shadow = asw::Color(0, 0, 0, static_cast<uint8_t>(200.0F * fade));
    auto color = t.color;
    color.a = static_cast<uint8_t>(255.0F * fade);
    asw::draw::text(font, t.text, screen + asw::Vec2f(2, 2), shadow,
                    asw::TextJustify::Center);
    asw::draw::text(font, t.text, screen, color, asw::TextJustify::Center);
  }
}

void Effects::burst(const asw::Vec2f& position,
                    const asw::Color& color,
                    int count,
                    float speed,
                    float size,
                    float gravity) {
  for (int i = 0; i < count && particles.size() < MAX_PARTICLES; i++) {
    const float angle =
        asw::random::between(0.0F, 2.0F * std::numbers::pi_v<float>);
    const float magnitude = asw::random::between(speed * 0.3F, speed);

    Particle p;
    p.position = position;
    p.velocity =
        asw::Vec2f(std::cos(angle) * magnitude, std::sin(angle) * magnitude);
    p.color = color;
    p.max_life = asw::random::between(0.35F, 0.8F);
    p.life = p.max_life;
    p.size = asw::random::between(size * 0.6F, size * 1.4F);
    p.gravity = gravity;
    particles.push_back(p);
  }
}

void Effects::ring(const asw::Vec2f& position,
                   float radius,
                   const asw::Color& color) {
  constexpr int count = 48;
  for (int i = 0; i < count && particles.size() < MAX_PARTICLES; i++) {
    const float angle =
        (static_cast<float>(i) / count) * 2.0F * std::numbers::pi_v<float>;
    const auto dir = asw::Vec2f(std::cos(angle), std::sin(angle) * 0.6F);

    Particle p;
    p.position = position + dir * (radius * 0.2F);
    p.velocity = dir * (radius * 3.5F);
    p.color = color;
    p.max_life = 0.45F;
    p.life = p.max_life;
    p.size = 10.0F;
    particles.push_back(p);
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
