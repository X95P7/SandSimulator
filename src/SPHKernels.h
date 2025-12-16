#pragma once
#include <cmath>

// Collection of SPH kernel helpers, grouped to keep FluidSimulation lean.
namespace SPHKernels {
// Spiky (power 2) kernel used for density (matches previous smoothingKernel)
double spikyPow2(double h, double distance);
double spikyPow2Derivative(float h, float distance);

// Spiky power 3 variant (often used for “near” density / pressure)
double spikyPow3(double h, double distance);
double spikyPow3Derivative(double h, double distance);

// Poly6 kernel (classic SPH) useful for viscosity or density sampling
double poly6(double h, double distance);
} // namespace SPHKernels

