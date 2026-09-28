#pragma once

#include <asw/asw.h>
#include <string>
#include <vector>

// Emitter shared by every burst with the same look
struct EmitterPool {
  asw::Color color;
  float speed{0.0F};
  float size{0.0F};
  float gravity{0.0F};
  bool ring{false};
  asw::ParticleEmitter emitter;
};

// Tumbling chunk of rubble that bounces once it hits the ground
struct Debris {
  asw::Vec2f position;
  asw::Vec2f velocity;
  asw::Vec2f size;
  asw::Color color;
  float angle{0.0F};
  float spin{0.0F};
  float ground{0.0F};
  float life{0.0F};
  float max_life{1.0F};
};

// Text that floats up and fades out
struct FloatingText {
  asw::Vec2f position;
  std::string text;
  asw::Color color;
  float life{0.0F};
  float max_life{1.0F};
  bool big{false};
};

// Sprite left on the ground that fades after a while
struct Decal {
  asw::Texture texture;
  asw::Quadf transform;
  float life{0.0F};
};

// Owns all short lived visual effects in a level
class Effects {
 public:
  void init(const asw::Font& small_font, const asw::Font& big_font);
  void clear();

  void update(float dt);

  // Draw ground decals, below everything else
  void draw_ground(const asw::Camera& camera) const;

  // Draw particles and text, above world objects
  void draw_top(const asw::Camera& camera) const;

  // Burst of particles
  void burst(const asw::Vec2f& position,
             const asw::Color& color,
             int count,
             float speed,
             float size = 5.0F,
             float gravity = 0.0F);

  // Ring of dust, used for the hammer slam
  void ring(const asw::Vec2f& position, float radius, const asw::Color& color);

  // Spinning chunks thrown up that land around the ground line
  void debris(const asw::Vec2f& position,
              float ground,
              const asw::Color& color,
              int count);

  void text(const asw::Vec2f& position,
            const std::string& text,
            const asw::Color& color,
            bool big = false);

  void decal(const asw::Texture& texture, const asw::Quadf& transform);

 private:
  // Find or make the emitter for a look
  asw::ParticleEmitter& emitter_for(const asw::Color& color,
                                    float speed,
                                    float size,
                                    float gravity,
                                    bool ring);

  // Emitters are drawn through the camera, which is not const
  mutable std::vector<EmitterPool> emitters;
  std::vector<Debris> debris_list;
  std::vector<FloatingText> texts;
  std::vector<Decal> decals;

  asw::Font small_font;
  asw::Font big_font;
};
