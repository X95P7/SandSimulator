#pragma once
#include "Vec2.h"
#include "Particle.h"
#include <vector>

class FluidSimulation {
private:
    std::vector<Particle> particles;
    Vec2 gravity;           // Gravity vector
    float timeStep;
    float top_border;
    float bottom_border;
    float left_border;
    float right_border;
    float damping;
    // SPH parameter (was a global constant); now configurable at runtime
    double smoothingRadius;

public:
    FluidSimulation(int count);
    FluidSimulation(int rows, int cols, float spacing, const Vec2& origin);
    void update();
    
    // Kernel functions
    double smoothingKernel(double r, double distance) const;              // Poly6 kernel for density
    double smoothingKernalDerivative(float r, float dst) const;           // (note: matches current implementation spelling)
    double spikyKernelDerivative(double r);       // Spiky kernel derivative for pressure gradient
    double viscosityLaplacian(double r);          // Viscosity kernel laplacian
    
    // Density and pressure
    double densityOf(const Particle& particle);
    double pressureOf(double density);
    double densityAt(float x, float y) const; // Density at arbitrary position
    double densityAtFast(float x, float y) const; // Faster variant (no sqrt)
    double getRestDensity() const; // Expected density (rho0)
    Vec2 calculateGradient(const Particle& particle);
    
    //Helper funtions
    void resolveCollisions(Particle& pi);
    
    // Getters
    const std::vector<Particle>& getPositions() const;
    // Gravity access
    const Vec2& getGravity() const { return gravity; }
    void setGravity(const Vec2& g) { gravity = g; }

    // Smoothing radius access
    double getSmoothingRadius() const { return smoothingRadius; }
    void setSmoothingRadius(double h) { smoothingRadius = std::max(1e-6, h); }
};
