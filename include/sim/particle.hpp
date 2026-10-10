#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "window.hpp"

struct Particle {
    lle::Vec2<f32_t> pos;
    f32_t radius;
    lle::Vec3<u8_t> color;
    bool hydrophobic;

    void Draw(lle::Window *w, lle::Vec2<f32_t> origin, f32_t scale) {
        w->DrawCircle({(origin.X + pos.X) * scale, (origin.Y + pos.Y) * scale}, radius * scale, color, lle::ShaderTypes::GRADIENT);
    }
};

#endif // !PARTICLE_HPP
