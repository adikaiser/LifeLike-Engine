#ifndef COMPOUND_HPP
#define COMPOUND_HPP

#include "types.hpp"
#include "vector.hpp"
#include "window.hpp"
#include "particle.hpp"
#include "parameters.hpp"
#include "functions.hpp"
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <string>
#include <unordered_map>
#include <functional>

enum CompoundType : u32_t;
enum CompoundInteractionType : u32_t;

inline CompoundType operator&(CompoundType l, CompoundType r) {
    return (CompoundType)((u32_t)l & (u32_t)r);
}

inline CompoundType operator|(CompoundType l, CompoundType r) {
    return (CompoundType)((u32_t)l | (u32_t)r);
}

#define NO_INTERACTION_TYPE (CompoundInteractionType)0

class Compound;
inline std::set<Compound*> deleteElems{};

class Compound {
public:
    static std::unordered_map<std::string, Compound> *templates;
    static inline f64_t totalLinVel;

    CompoundType type;
    lle::Vec2<f32_t> pos;
    lle::Vec2<f64_t> linearVel;
    f64_t angularVel;
    std::vector<Particle> particles;
    std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> onContact;
    u32_t interactsWith;
    std::vector<std::string> encodes;
    f64_t lifetime;
    u64_t spawnTime;
    bool moved;

    void adjustCoM() noexcept {

        // Translate particles' relative position so that pos is the center of mass
        f64_t cmX = 0, cmY = 0;
        for (size_t i = 0; i < particles.size(); i++) {
            cmX += particles[i].pos.X;
            cmY += particles[i].pos.Y;
        }
        cmX /= particles.size();
        cmY /= particles.size();

        for (auto &particle : particles) {
            particle.pos.X -= cmX;
            particle.pos.Y -= cmY;
        }
    }

    Compound(CompoundType type, lle::Vec2<f32_t> pos, std::vector<Particle> particles, std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> onContact={}, std::vector<std::string> encodes={}, CompoundType interaction=(CompoundType)0x7FFFFFFF, f64_t lifetime=-1, lle::Vec2<f64_t> linVel={0.0, 0.0}, f64_t angVel=0.0, u64_t spawnTime=0) 
        : type(type),
          pos(pos),
          linearVel(linVel),
          angularVel(angVel),
          particles(particles),
          onContact(onContact),
          interactsWith(static_cast<u32_t>(interaction)),
          encodes(encodes),
          lifetime(lifetime),
          spawnTime(spawnTime),
          moved(false) {
        adjustCoM();
        if (lifetime < 0) {
            lifetime = std::numeric_limits<f64_t>::infinity();
        }
        if (spawnTime == 0) {
            spawnTime = frameCounter;
            globalTimer.recordNow();
        }
    }

    Compound(const Compound&) = default;
    Compound& operator=(const Compound&) = default;

    Compound(Compound&& other) noexcept
        : type(other.type),
          pos(std::move(other.pos)),
          linearVel(std::move(other.linearVel)),
          angularVel(other.angularVel),
          particles(std::move(other.particles)),
          onContact(std::move(other.onContact)),
          interactsWith(other.interactsWith),
          encodes(std::move(other.encodes)),
          lifetime(other.lifetime),
          spawnTime(other.spawnTime),
          moved(false) {
        other.moved = true;
    }

    Compound& operator=(Compound&& other) noexcept {
        if (this != &other)
        {
            type = std::move(other.type);
            pos = std::move(other.pos);
            linearVel = std::move(other.linearVel);
            angularVel = other.angularVel;
            particles = std::move(other.particles);
            onContact = std::move(other.onContact);
            interactsWith = other.interactsWith;
            encodes = std::move(other.encodes);
            lifetime = other.lifetime;
            spawnTime = other.spawnTime;

            moved = false;
            other.moved = true;
        }

        return *this;
    }

    Compound(const Compound &tmplt, lle::Vec2<f32_t> pos, lle::Vec2<f64_t> linVel={0.0, 0.0})
        : type(tmplt.type),
          pos(pos),
          particles(tmplt.particles),
          linearVel(linVel),
          angularVel(tmplt.angularVel),
          onContact(tmplt.onContact),
          interactsWith(tmplt.interactsWith),
          encodes(tmplt.encodes),
          lifetime(tmplt.lifetime),
          spawnTime(frameCounter),
          moved(false) {
        globalTimer.recordNow();
    }

    ~Compound() {
        if (!moved) {
            globalTimer.deleteEntry(spawnTime);
        }
    }
    
    static void Calculate(Compound &c1, Compound &c2, f64_t dt) {

        /*if (globalTimer.Read(c1.spawnTime) > c1.lifetime) {
            deleteElems.insert(&c1);
            if (globalTimer.Read(c2.spawnTime) > c2.lifetime) deleteElems.insert(&c2);
            return;
        } else if (globalTimer.Read(c2.spawnTime) > c2.lifetime) {
            deleteElems.insert(&c2);
            return;
        }*/

        const lle::Vec2<f32_t> relative{c2.pos.X - c1.pos.X, c2.pos.Y - c1.pos.Y};
        lle::Vec2<f64_t> totalLinForce{0.0, 0.0};
        f64_t c1TotalTorque = 0.0;
        f64_t c2TotalTorque = 0.0;
        for (size_t iC1 = 0; iC1 < c1.particles.size(); iC1++) {
            for (size_t iC2 = 0; iC2 < c2.particles.size(); iC2++) {
                Particle &c1Particle = c1.particles[iC1];
                Particle &c2Particle = c2.particles[iC2];
                f64_t currentForce = 0.0;
                const f64_t dx = (c2Particle.pos.X + relative.X) - c1Particle.pos.X;
                const f64_t dy = (c2Particle.pos.Y + relative.Y) - c1Particle.pos.Y;
                f64_t dist = std::sqrt(dx * dx + dy * dy);
                lle::Vec2<f32_t> particleRelativeNormalized{dx / dist, dy / dist};

                if (dist == 0.0) {
                    c1.pos.X += 0.01;
                    c2.pos.Y -= 0.01;
                    return;
                }

                const f64_t minDistance = c1Particle.radius + c2Particle.radius;
                const f64_t maxDistance = minDistance + maxInteractionDist;

                if (dist <= minDistance) {
                    currentForce -= ljForce(dist, 1.0, minDistance);
                } else if (dist <= maxDistance) {
                    currentForce -= ljForce(
                        dist,
                        (c1Particle.hydrophobic == c2Particle.hydrophobic) ? (c1Particle.hydrophobic ?
                            hydrophobicInteraction :
                            hydrohpilicInteraction) :
                            oppositeInteraction,
                        minDistance);
                }

                currentForce *= 1e-18;

                totalLinForce.X += particleRelativeNormalized.X * currentForce;
                totalLinForce.Y += particleRelativeNormalized.Y * currentForce;

                c1TotalTorque += 1e-9 * c1Particle.pos.X * particleRelativeNormalized.Y * currentForce - 
                                 1e-9 * c1Particle.pos.Y * particleRelativeNormalized.X * currentForce;

                c2TotalTorque += 1e-9 * c2Particle.pos.X * (-particleRelativeNormalized.Y) * currentForce - 
                                 1e-9 * c2Particle.pos.Y * (-particleRelativeNormalized.X) * currentForce; 

                if (dist <= minDistance + 0.1) {
                    if (c1.onContact.contains(c2.type)) c1.onContact.at(c2.type)(&c1, iC1, &c2, iC2);
                    if (c2.onContact.contains(c1.type)) c2.onContact.at(c1.type)(&c2, iC2, &c1, iC1);
                }
            }
        }

        if (c1.interactsWith & c2.type) {

            c1.linearVel.X += (totalLinForce.X / ldCoeff) * dt;
            c1.linearVel.Y += (totalLinForce.Y / ldCoeff) * dt;

            c1.angularVel += (c1TotalTorque / adCoeff) * dt;
        }

        if (c2.interactsWith & c1.type) {

            c2.linearVel.X -= (totalLinForce.X / ldCoeff) * dt;
            c2.linearVel.Y -= (totalLinForce.Y / ldCoeff) * dt;

            c2.angularVel += (c2TotalTorque / adCoeff) * dt;
        }

    }

    void Update(lle::Window *w, f64_t dt, bool updatePhysics=true) {

        if (!updatePhysics) {
            for (auto &particle : particles) {
                particle.Draw(w, {pos.X + simArea.X, pos.Y + simArea.Y}, simScale);
            }
            return;
        }

        if ((pos.X < 0 && linearVel.X < 0) || (pos.X > simArea.Z && linearVel.X > 0)) linearVel.X *= -1;
        if ((pos.Y < 0 && linearVel.Y < 0) || (pos.Y > simArea.W && linearVel.Y > 0)) linearVel.Y *= -1;

        f64_t speed = std::max(std::sqrt(linearVel.X * linearVel.X + linearVel.Y * linearVel.Y), 1.0e-10);
        f64_t newSpeed = speed;
        if (newSpeed > maxLinVel) {
            newSpeed = maxLinVel;
        }
        newSpeed += avgLinVel - lastAvgLinVel;
        totalLinVel += newSpeed;

        const f64_t factor = newSpeed / speed;
        linearVel.X *= factor;
        linearVel.Y *= factor;

        if (angularVel > maxAngVel) angularVel = maxAngVel;
        else if (angularVel < -maxAngVel) angularVel = -maxAngVel;

        pos.X += linearVel.X * dt * 1'000'000'000;
        pos.Y += linearVel.Y * dt * 1'000'000'000;

        const f32_t c = std::cos(angularVel * dt);
        const f32_t s = std::sin(angularVel * dt);

        for (auto &particle : particles) {

            const f32_t x = particle.pos.X;
            const f32_t y = particle.pos.Y;

            particle.pos.X = x * c - y * s;
            particle.pos.Y = x * s + y * c;

            particle.Draw(w, {pos.X + simArea.X, pos.Y + simArea.Y}, simScale);
        }
    }
};

inline std::vector<Compound> compounds{};
inline std::vector<Compound> addElems{};

#endif // !COMPOUND_HPP
