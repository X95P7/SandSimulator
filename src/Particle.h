#ifndef PARTICLE_H
#define PARTICLE_H

#include <cmath>

class Particle {
private:
    double x, y;           // Position
    double vx, vy;         // Velocity
    double mass;           // Mass
    bool active;           // Whether particle is active/alive

public:
    // Constructors
    Particle();
    Particle(double x, double y, double vx, double vy, double mass);
    
    // Getters
    double getX() const { return x; }
    double getY() const { return y; }
    double getVx() const { return vx; }
    double getVy() const { return vy; }
    double getMass() const { return mass; }
    bool isActive() const { return active; }
    
    // Setters
    void setPosition(double x, double y);
    void setVelocity(double vx, double vy);
    void setMass(double mass);
    void setActive(bool active);
    
    // Physics methods
    void update(double dt);                    // Update position based on velocity
    void applyForce(double fx, double fy, double dt);     // Apply force to particle
    
    // Utility methods
    double distanceTo(const Particle& other) const;
    
    // Destructor
    ~Particle() = default;
};

#endif // PARTICLE_H
