#include "FluidSimulation.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

// -------------------- SPH Constants (tweak these) --------------------
const double PARTICLE_RADIUS       = 0.02;
const double SMOOTHING_RADIUS      = 0.11;        // h (default)
const double TARGET_DENSITY          = 1.5;     // rho0
const double PRESSURE_MULTIPLIER    = 1.0;  
const double VISCOSITY_CONSTANT   = 0.01;         // mu (typical SPH friendly value)
const double SURFACE_TENSION_CONSTANT = 0.0000001;  // optional
const double EPSILON              = 1e-6;        // small value to avoid div-by-zero

// Precomputed kernel normalization constants (for convenience)
const double M_PI = 3.14159265358979323846;
// Note: these depend on h; compute on the fly instead of fixing at startup

// --------------------------------------------------------------------

FluidSimulation::FluidSimulation(int count)
    : gravity(0.0, -4.0),    // gravity Y approx -4 (from provided settings)
      timeStep(0.005f),
      top_border(1.0), bottom_border(-1.0),
      left_border(-1.0), right_border(1.0),
      damping(0.5f),
      velocityDrag(0.99f),   // per-step velocity drag
      collisionDamping(0.0f),
      smoothingRadius(0.16433),
      pressureMultiplier(4.12456),
      nearPressureMultiplier(0.93206),
      viscosityStrength(0.0),
      restDensity(5.0),
      maxVelocity(2.01)
{
    particles.reserve(count);
    for (int i = 0; i < count; ++i) {
        float x = (float(rand()) / RAND_MAX) * 1.6f - 0.8f; // cluster toward center
        float y = (float(rand()) / RAND_MAX) * 0.8f + 0.0f; // place them in upper half to fall
        float vx = ((float(rand()) / RAND_MAX) * 2 - 1) * 0.01f;
        float vy = 0.0f;

        // mass of each particle: choose something reasonable (mass affects acceleration)
        double mass = 1;
        particles.push_back(Particle(x, y, vx, vy, mass));
    }
}

FluidSimulation::FluidSimulation(int rows, int cols, float spacing, const Vec2& origin)
    : gravity(0.0, -4.0),
      timeStep(0.005f),
      top_border(1.0), bottom_border(-1.0),
      left_border(-1.0), right_border(1.0),
      damping(0.5f),
      velocityDrag(0.99f),
      collisionDamping(0.0f),
      smoothingRadius(0.16433),
      pressureMultiplier(4.12456),
      nearPressureMultiplier(0.93206),
      viscosityStrength(0.0),
      restDensity(5.0),
      maxVelocity(2.01)
{
    int total = rows * cols;
    particles.reserve(total);
    const double mass = 1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float x = origin.x + c * spacing;
            float y = origin.y + r * spacing;

            if (x < left_border) x = (float)left_border;
            if (x > right_border) x = (float)right_border;
            if (y < bottom_border) y = (float)bottom_border;
            if (y > top_border) y = (float)top_border;

            particles.emplace_back(x, y, 0.0, 0.0, mass);
        }
    }
}

// -------------------- Kernels --------------------

double FluidSimulation::smoothingKernel(double r, double distance) const {
    if (distance > r) {
        return 0.0;
    }

    float volume = M_PI * pow(r ,4) / 6;
    return (r - distance) * (r - distance) / volume;
}

double FluidSimulation::smoothingKernalDerivative(float r, float dst) const {
    if(dst > r){
        return 0.0;
    }
    float scale = 12 / (M_PI * pow(r, 4));
    return (dst - r) * scale;
}

float FluidSimulation::calculateSharedPressure(float densityA, float densityB) {
    float pressureA = pressureOf(densityA);
    float pressureB = pressureOf(densityB);
    return (pressureA + pressureB) / 2;
}

Vec2 FluidSimulation::calculateGradient(const Particle& particle) {
    Vec2 point = Vec2(particle.getX(), particle.getY());
    Vec2 gradient(0.0, 0.0);
    double thisDensity = particle.getDensity();

    for (int i = 0; i < particles.size(); i++) {
        const Particle& otherParticle = particles[i];
        if (&otherParticle == &particle) continue; // skip self

        Vec2 other = Vec2(otherParticle.getX(), otherParticle.getY());
        Vec2 r = point - other;
        double dst = r.magnitude();

        if (dst < smoothingRadius && dst > 0.0) {
            Vec2 direction = r.normalized();
            double slope = smoothingKernalDerivative((float)smoothingRadius, (float)dst); // dW/dr
            double mass = otherParticle.getMass();
            double density = otherParticle.getDensity();
            double sharedPressure = calculateSharedPressure(thisDensity, density); // e.g. pressure or temperature

            // ∇A_i += m_j * (A_j / ρ_j) * ∇W(r_ij, h)
            float scale = (float)(-slope * mass * sharedPressure / density);
            gradient += direction * scale;
        }
    }

    return gradient;
}

// -------------------- Density & Pressure --------------------

// Compute density for a single particle (sum over neighbors)
double FluidSimulation::densityOf(const Particle& particle) {
    double density = 0.0;

    for (const auto& neighbor : particles) {
            double dist = particle.distanceTo(neighbor);
            double influence = smoothingKernel(smoothingRadius, dist);
            density += neighbor.getMass() * influence;
    }
    // avoid zero density
    return std::max(density, EPSILON);
}

// Pressure 
double FluidSimulation::pressureOf(double density) {
    double p = pressureMultiplier * (density - restDensity);
    return std::max(p, 0.0);
}


// -------------------- Update --------------------

void FluidSimulation::update() {
    size_t N = particles.size();
    Vec2 graivityForce = (gravity);
    if (N == 0) return;

    // 1) Compute densities and pressures for all particles (stored in objects)
    for (size_t i = 0; i < N; ++i) {
        double density = densityOf(particles[i]);
        particles[i].setDensity(density);
        //particles[i].setPressure(pressureOf(density));
    } 

    for (size_t i = 0; i < N; ++i) {
        Particle& pi = particles[i];
        Vec2 pressureForce = calculateGradient(pi);
        Vec2 pressureAcceleration = pressureForce / pi.getDensity();
        pi.applyForce(pressureAcceleration.x + graivityForce.x, pressureAcceleration.y + graivityForce.y, timeStep);
    }

    // 2) Move paricles
    for (size_t i = 0; i < N; ++i) {
        Particle& pi = particles[i];

        // Apply per-step velocity drag to help particles settle
        double vx = pi.getVx();
        double vy = pi.getVy();
        vx *= velocityDrag;
        vy *= velocityDrag;
        pi.setVelocity(vx, vy);

        // Integrate position
        pi.update(timeStep);
        resolveCollisions(pi);

    }
}

void FluidSimulation::resolveCollisions(Particle& pi) {
    if (pi.getY() < bottom_border) {
        pi.setPosition(pi.getX(), bottom_border);
        pi.setVelocity(pi.getVx(), -pi.getVy() * damping);
    }
    if (pi.getY() > top_border) {
        pi.setPosition(pi.getX(), top_border);
        pi.setVelocity(pi.getVx(), -pi.getVy() * damping);
    }
    if (pi.getX() < left_border) {
        pi.setPosition(left_border, pi.getY());
        pi.setVelocity(-pi.getVx() * damping, pi.getVy());
    }
    if (pi.getX() > right_border) {
        pi.setPosition(right_border, pi.getY());
        pi.setVelocity(-pi.getVx() * damping, pi.getVy());
    }
}
const std::vector<Particle>& FluidSimulation::getPositions() const {
    return particles;
}

double FluidSimulation::densityAt(float x, float y) const {
    double density = 0.0;
    for (const auto& neighbor : particles) {
        double dx = static_cast<double>(x) - neighbor.getX();
        double dy = static_cast<double>(y) - neighbor.getY();
        double dist = std::sqrt(dx * dx + dy * dy);
        double influence = smoothingKernel(smoothingRadius, dist);
        density += neighbor.getMass() * influence;
    }
    return std::max(density, EPSILON);
}

double FluidSimulation::densityAtFast(float x, float y, double smoothingRadius) const {
    // Compute using squared distance to avoid sqrt and inline Poly6
    const double h = smoothingRadius;
    const double h2 = h * h;
    const double volume = M_PI * pow(h, 8) / 4.0; // matches smoothingKernel
    double density = 0.0;
    for (const auto& neighbor : particles) {
        const double dx = static_cast<double>(x) - neighbor.getX();
        const double dy = static_cast<double>(y) - neighbor.getY();
        const double r2 = dx * dx + dy * dy;
        const double t = h2 - r2;
        if (t > 0.0) {
            const double w = (t * t * t) / volume; // Poly6
            density += neighbor.getMass() * w;
        }
    }
    return std::max(density, EPSILON);
}

// getRestDensity() is now inline in the header

// Apply mouse interaction force (positive strength = attract, negative = repel)
void FluidSimulation::applyInteraction(const Vec2& point, double strength, double radius) {
    if (strength == 0.0 || radius <= 0.0) return;
    const double r2 = radius * radius;
    for (auto& p : particles) {
        double dx = p.getX() - point.x;
        double dy = p.getY() - point.y;
        double dist2 = dx * dx + dy * dy;
        if (dist2 > r2 || dist2 < EPSILON) continue;
        double dist = std::sqrt(dist2);
        double falloff = 1.0 - (dist / radius); // linear falloff
        Vec2 dir(dx / dist, dy / dist);
        // Attract (strength > 0) toward point, repel (strength < 0) away from point
        // Boost interaction strength to ensure noticeable motion
        const double boost = 5.0;
        double fx = -strength * falloff * dir.x * boost;
        double fy = -strength * falloff * dir.y * boost;
        p.applyForce(fx, fy, timeStep);
    }
}

// Reset particles with custom spawn settings
void FluidSimulation::resetParticles(int count, float spreadX, float spreadY, float originX, float originY) {
    particles.clear();
    particles.reserve(count);

    const double mass = 1.0;
    for (int i = 0; i < count; ++i) {
        // Position within the spread area centered at origin
        float x = originX + (float(rand()) / RAND_MAX - 0.5f) * spreadX;
        float y = originY + (float(rand()) / RAND_MAX - 0.5f) * spreadY;

        // Clamp to borders
        if (x < left_border) x = (float)left_border;
        if (x > right_border) x = (float)right_border;
        if (y < bottom_border) y = (float)bottom_border;
        if (y > top_border) y = (float)top_border;

        // Small random initial velocity
        float vx = ((float(rand()) / RAND_MAX) * 2 - 1) * 0.01f;
        float vy = 0.0f;

        particles.emplace_back(x, y, vx, vy, mass);
    }
}
