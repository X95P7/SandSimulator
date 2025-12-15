#pragma once
#include "Vec2.h"
#include "Particle.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

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
    float velocityDrag;      // Per-step velocity drag (0-1)
    float collisionDamping;  // Separate damping for collisions (like Unity Sim 2D)
    // SPH parameter (was a global constant); now configurable at runtime
    double smoothingRadius;
    double pressureMultiplier;
    double nearPressureMultiplier;  // Near pressure multiplier for dual density SPH
    double viscosityStrength;        // Viscosity strength constant
    double restDensity;  // TARGET_DENSITY (rho0)
    double maxVelocity;  // Maximum velocity clamp
    
    // Spatial hash for neighbor search optimization
    struct CellCoord {
        int x, y;
        bool operator==(const CellCoord& other) const {
            return x == other.x && y == other.y;
        }
    };
    struct CellCoordHash {
        std::size_t operator()(const CellCoord& c) const {
            const uint32_t hashK1 = 15823;
            const uint32_t hashK2 = 9737333;
            return static_cast<uint32_t>(c.x) * hashK1 + static_cast<uint32_t>(c.y) * hashK2;
        }
    };
    std::unordered_map<CellCoord, std::vector<size_t>, CellCoordHash> spatialGrid;
    
    // Spatial hash helper functions
    CellCoord getCellCoord(double x, double y) const;
    void buildSpatialGrid();
    void getNeighbors(size_t particleIndex, std::vector<size_t>& neighbors) const;
public:
    FluidSimulation(int count);
    FluidSimulation(int rows, int cols, float spacing, const Vec2& origin);
    void update();
    
    // Kernel functions
    double smoothingKernel(double r, double distance) const;              // SpikyPow2 kernel for density
    double smoothingKernalDerivative(float r, float dst) const;           // DerivativeSpikyPow2 (note: matches current implementation spelling)
    double spikyKernelPow3(double r, double distance) const;               // SpikyPow3 kernel for near density
    double derivativeSpikyPow3(double r, double distance) const;         // DerivativeSpikyPow3 for near pressure
    double poly6Kernel(double r, double distance) const;                  // Poly6 kernel for viscosity
    
    // Density and pressure
    double densityOf(const Particle& particle);
    double nearDensityOf(const Particle& particle);  // Near density calculation
    double pressureOf(double density);
    double nearPressureOf(double nearDensity);       // Near pressure calculation
    double densityAt(float x, float y) const; // Density at arbitrary position
    double densityAtFast(float x, float y, double smoothingRadius) const; // Faster variant (no sqrt)
    Vec2 calculateGradient(const Particle& particle);
    Vec2 calculateViscosity(const Particle& particle);  // Viscosity force calculation
    float calculateSharedPressure(float densityA, float densityB);
    float calculateSharedNearPressure(float nearDensityA, float nearDensityB);
    
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

    // Pressure multiplier access
    double getPressureMultiplier() const { return pressureMultiplier; }
    void setPressureMultiplier(double p) { pressureMultiplier = std::max(1e-6, p); }

    // Time step access
    float getTimeStep() const { return timeStep; }
    void setTimeStep(float dt) { timeStep = std::max(1e-6f, dt); }

    // Damping access
    float getDamping() const { return damping; }
    void setDamping(float d) { damping = std::max(0.0f, std::min(1.0f, d)); }
    // Velocity drag access
    float getVelocityDrag() const { return velocityDrag; }
    void setVelocityDrag(float d) { velocityDrag = std::max(0.0f, std::min(1.0f, d)); }
    
    // Collision damping access
    float getCollisionDamping() const { return collisionDamping; }
    void setCollisionDamping(float d) { collisionDamping = std::max(0.0f, std::min(1.0f, d)); }

    // Rest density access
    double getRestDensity() const { return restDensity; }
    void setRestDensity(double rho) { restDensity = std::max(1e-6, rho); }

    // Near pressure multiplier access
    double getNearPressureMultiplier() const { return nearPressureMultiplier; }
    void setNearPressureMultiplier(double p) { nearPressureMultiplier = std::max(1e-6, p); }

    // Viscosity strength access
    double getViscosityStrength() const { return viscosityStrength; }
    void setViscosityStrength(double v) { viscosityStrength = std::max(0.0, v); }
    
    // External interaction (mouse) force
    void applyInteraction(const Vec2& point, double strength, double radius);

    // Max velocity access
    double getMaxVelocity() const { return maxVelocity; }
    void setMaxVelocity(double v) { maxVelocity = std::max(0.0, v); }
    
    // Reset particles with custom spawn settings
    void resetParticles(int count, float spreadX, float spreadY, float originX, float originY);
};
