#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "types.hpp"
#include "vector.hpp"
#include "parameters.hpp"

inline lle::Vec2<f32_t> transform(lle::Vec2<f32_t> v) {
    return {v.X / simScale - simArea.X, v.Y / simScale - simArea.Y};
}

// Function from ChatGPT
constexpr f64_t ljForce(f64_t r, f64_t epsilon, f64_t sigma) {
    double sr = sigma / r;
    double sr2 = sr * sr;        // (σ/r)^2
    double sr6 = sr2 * sr2 * sr2; // (σ/r)^6
    double sr12 = sr6 * sr6;      // (σ/r)^12

    double force = (24.0 * epsilon / r) * (2.0 * sr12 - sr6);

    return force; // scalar magnitude
}

#endif // !FUNCTIONS_HPP
