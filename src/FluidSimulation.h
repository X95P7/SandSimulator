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

public:
    FluidSimulation(int count);
    void update();
    
    // Kernel functions
    double smoothingKernel(double r) const;              // Poly6 kernel for density
    double spikyKernelDerivative(double r);       // Spiky kernel derivative for pressure gradient
    double viscosityLaplacian(double r);          // Viscosity kernel laplacian
    
    // Density and pressure
    double densityOf(const Particle& particle);
    double pressureFromDensity(double density);
    double densityAt(float x, float y) const; // Density at arbitrary position
    double getRestDensity() const; // Expected density (rho0)
    
    // Forces
    Vec2 viscosityOf(const Particle& pi, size_t iIndex, const std::vector<double>& densities);
    
    // Getters
    const std::vector<Particle>& getPositions() const;
};
