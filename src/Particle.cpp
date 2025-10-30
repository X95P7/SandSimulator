#include "Particle.h"

// Default constructor
Particle::Particle() 
    : x(0.0), y(0.0), vx(0.0), vy(0.0), mass(1.0), active(true) {
}

// Parameterized constructor
Particle::Particle(double x, double y, double vx, double vy, double mass)
    : x(x), y(y), vx(vx), vy(vy), mass(mass), active(true) {
}

// Setters
void Particle::setPosition(double x, double y) {
    this->x = x;
    this->y = y;
}

void Particle::setVelocity(double vx, double vy) {
    this->vx = vx;
    this->vy = vy;
}

void Particle::setMass(double mass) {
    this->mass = mass;
}

void Particle::setActive(bool active) {
    this->active = active;
}

// Physics methods
void Particle::update(double dt) {
    if (!active) return;
    
    // Update position based on velocity
    x += vx * dt;
    y += vy * dt;
}

void Particle::applyForce(double fx, double fy, double dt) {
    if (!active) return;
    
    // F = ma, so a = F/m
    double ax = fx / mass;
    double ay = fy / mass;
    
    // Update velocity: v = v0 + a * dt
    vx += ax * dt;
    vy += ay * dt;
}

// Utility methods
double Particle::distanceTo(const Particle& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}