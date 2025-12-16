#include "SPHKernels.h"

namespace {
// Pi constant local to this TU
constexpr double kPi = 3.14159265358979323846;
}

namespace SPHKernels {

double spikyPow2(double h, double distance) {
    if (distance > h || h <= 0.0) return 0.0;
    // Matches previous implementation: (h - r)^2 / (pi * h^4 / 6)
    double volume = kPi * std::pow(h, 4) / 6.0;
    double t = (h - distance);
    return (t * t) / volume;
}

double spikyPow2Derivative(float h, float distance) {
    if (distance > h || h <= 0.0f) return 0.0;
    // Derivative of above w.r.t distance
    float scale = 12.0f / (static_cast<float>(kPi) * std::pow(h, 4));
    return (distance - h) * scale;
}

double spikyPow3(double h, double distance) {
    if (distance > h || h <= 0.0) return 0.0;
    // Common form: (h - r)^3 / (pi * h^6)
    double volume = kPi * std::pow(h, 6);
    double t = (h - distance);
    return (t * t * t) / volume;
}

double spikyPow3Derivative(double h, double distance) {
    if (distance > h || h <= 0.0) return 0.0;
    double volume = kPi * std::pow(h, 6);
    double t = (h - distance);
    // d/dr of (h - r)^3 / volume = -3(h - r)^2 / volume
    return -3.0 * t * t / volume;
}

double poly6(double h, double distance) {
    if (distance >= h || h <= 0.0) return 0.0;
    // Classic poly6: 315/(64*pi*h^9) * (h^2 - r^2)^3
    double h2 = h * h;
    double t = h2 - distance * distance;
    const double factor = 315.0 / (64.0 * kPi * std::pow(h, 9));
    return factor * t * t * t;
}

} // namespace SPHKernels

