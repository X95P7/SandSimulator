#include "FluidSimulation.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

// -------------------- SPH Constants (tweak these) --------------------
const double PARTICLE_RADIUS       = 0.02;
const double SMOOTHING_RADIUS      = 0.05;        // h (default)
const double TARGET_DENSITY          = 1.0;     // rho0
const double PRESSURE_MULTIPLIER    = 1.0;  
const double VISCOSITY_CONSTANT   = 0.01;         // mu (typical SPH friendly value)
const double SURFACE_TENSION_CONSTANT = 0.0000001;  // optional
const double EPSILON              = 1e-6;        // small value to avoid div-by-zero

// Precomputed kernel normalization constants (for convenience)
const double M_PI = 3.14159265358979323846;
// Note: these depend on h; compute on the fly instead of fixing at startup

// --------------------------------------------------------------------

FluidSimulation::FluidSimulation(int count)
    : gravity(0.0, -0.005), // standard gravity direction (y)
      timeStep(0.005),    // smaller time step -> more stable (tweak: 0.003-0.01)
      top_border(1.0), bottom_border(-1.0),
      left_border(-1.0), right_border(1.0),
      damping(0.9),
      smoothingRadius(SMOOTHING_RADIUS)
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
    : gravity(0.0, -0.005),
      timeStep(0.005),
      top_border(1.0), bottom_border(-1.0),
      left_border(-1.0), right_border(1.0),
      damping(0.9),
      smoothingRadius(SMOOTHING_RADIUS)
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
    float volume = M_PI * pow(r ,8) / 4;
    float value = std::max(0.0f, static_cast<float>(r * r - distance * distance));
    return value * value * value / volume;
}

double FluidSimulation::smoothingKernalDerivative(float r, float dst) const {
    if(dst > r){
        return 0.0;
    }
    float f = r * r - dst * dst;
    float scale = -24 / (M_PI * pow(r, 8));
    return scale * dst * f * f;
}

Vec2 FluidSimulation::calculateGradient(const Particle& particle) {
    Vec2 point = Vec2(particle.getX(), particle.getY());
    Vec2 gradient(0.0, 0.0);

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
            double pressure = otherParticle.getPressure(); // e.g. pressure or temperature

            // ∇A_i += m_j * (A_j / ρ_j) * ∇W(r_ij, h)
            float scale = (float)(-slope * mass * pressure / density);
            gradient += direction * scale;
        }
    }

    return gradient;
}

// Spiky kernel derivative magnitude (dW/dr) for pressure gradient
double FluidSimulation::spikyKernelDerivative(double r) {
    if (r <= 0.0 || r >= smoothingRadius) return 0.0;
    double hr = (smoothingRadius - r);
    // dW/dr for spiky: -45/(pi h^6) * (h - r)^2
    // Here we return the scalar dW/dr (note: negative already included)
    return -45.0 / (M_PI * pow(smoothingRadius, 6)) * hr * hr;
    // Alternatively: return -SPIKY_FACTOR * 3 * pow((SMOOTHING_RADIUS - r), 2);
}

// Viscosity kernel laplacian (scalar)
double FluidSimulation::viscosityLaplacian(double r) {
    if (r >= smoothingRadius) return 0.0;
    double hr = (smoothingRadius - r);
    // Laplacian of viscosity kernel: 45/(pi h^6) * (h - r)
    return (45.0 / (M_PI * pow(smoothingRadius, 6))) * hr;
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
    double p = PRESSURE_MULTIPLIER * (density - TARGET_DENSITY);
    return std::max(p, 0.0);
}


// -------------------- Update --------------------

void FluidSimulation::update() {
    size_t N = particles.size();
    if (N == 0) return;

    // 1) Compute densities and pressures for all particles (stored in objects)
    for (size_t i = 0; i < N; ++i) {
        double density = densityOf(particles[i]);
        particles[i].setDensity(density);
        particles[i].setPressure(pressureOf(density));

        Vec2 totalForce = (gravity);
        particles[i].applyForce(totalForce.x, totalForce.y, timeStep);
    } 

    for (size_t i = 0; i < N; ++i) {
        Particle& pi = particles[i];
        Vec2 pressureForce = calculateGradient(pi);
        Vec2 pressureAcceleration = pressureForce / pi.getDensity();
        pi.applyForce(pressureAcceleration.x, pressureAcceleration.y, timeStep);
    }

    // 2) Move paricles
    for (size_t i = 0; i < N; ++i) {
        Particle& pi = particles[i];

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

double FluidSimulation::densityAtFast(float x, float y) const {
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

double FluidSimulation::getRestDensity() const {
    return TARGET_DENSITY;
}
