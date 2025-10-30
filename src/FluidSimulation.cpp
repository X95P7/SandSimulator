#include "FluidSimulation.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

// -------------------- SPH Constants (tweak these) --------------------
const double PARTICLE_RADIUS       = 0.02;
const double SMOOTHING_RADIUS      = 0.05;        // h
const double REST_DENSITY          = 1.0;     // rho0
const double GAS_CONSTANT         = 50.0;      // k (stiffness) — you can reduce for softness
const double VISCOSITY_CONSTANT   = 0.01;         // mu (typical SPH friendly value)
const double SURFACE_TENSION_CONSTANT = 0.0000001;  // optional
const double EPSILON              = 1e-6;        // small value to avoid div-by-zero

// Precomputed kernel normalization constants (for convenience)
const double POLY6_FACTOR  = 315.0 / (64.0 * M_PI * pow(SMOOTHING_RADIUS, 9));
const double SPIKY_FACTOR  = 15.0  / (M_PI * pow(SMOOTHING_RADIUS, 6)); // used below (we multiply appropriately)
const double VISC_LAPLACIAN_FACTOR = 45.0 / (M_PI * pow(SMOOTHING_RADIUS, 6));

// --------------------------------------------------------------------

FluidSimulation::FluidSimulation(int count)
    : gravity(0.0, -98.0), // standard gravity direction (y)
      timeStep(0.005),    // smaller time step -> more stable (tweak: 0.003-0.01)
      top_border(1.0), bottom_border(-1.0),
      left_border(-1.0), right_border(1.0),
      damping(0.9)
{
    particles.reserve(count);
    for (int i = 0; i < count; ++i) {
        float x = (float(rand()) / RAND_MAX) * 1.6f - 0.8f; // cluster toward center
        float y = (float(rand()) / RAND_MAX) * 0.8f + 0.0f; // place them in upper half to fall
        float vx = ((float(rand()) / RAND_MAX) * 2 - 1) * 0.01f;
        float vy = 0.0f;

        // mass of each particle: choose something reasonable (mass affects acceleration)
        double mass = 0.01;
        particles.push_back(Particle(x, y, vx, vy, mass));
    }
}

// -------------------- Kernels --------------------

// Poly6 kernel for density (scalar)
double FluidSimulation::smoothingKernel(double r) const {
    if (r >= SMOOTHING_RADIUS) return 0.0;
    double diff = (SMOOTHING_RADIUS * SMOOTHING_RADIUS - r * r);
    return POLY6_FACTOR * diff * diff * diff;
}

// Spiky kernel derivative magnitude (dW/dr) for pressure gradient
double FluidSimulation::spikyKernelDerivative(double r) {
    if (r <= 0.0 || r >= SMOOTHING_RADIUS) return 0.0;
    double hr = (SMOOTHING_RADIUS - r);
    // dW/dr for spiky: -45/(pi h^6) * (h - r)^2
    // Here we return the scalar dW/dr (note: negative already included)
    return -45.0 / (M_PI * pow(SMOOTHING_RADIUS, 6)) * hr * hr;
    // Alternatively: return -SPIKY_FACTOR * 3 * pow((SMOOTHING_RADIUS - r), 2);
}

// Viscosity kernel laplacian (scalar)
double FluidSimulation::viscosityLaplacian(double r) {
    if (r >= SMOOTHING_RADIUS) return 0.0;
    double hr = (SMOOTHING_RADIUS - r);
    // Laplacian of viscosity kernel: 45/(pi h^6) * (h - r)
    return VISC_LAPLACIAN_FACTOR * hr;
}

// -------------------- Density & Pressure --------------------

// Compute density for a single particle (sum over neighbors)
double FluidSimulation::densityOf(const Particle& particle) {
    double density = 0.0;
    for (const auto& neighbor : particles) {
        if (&neighbor != &particle) {
            double dist = particle.distanceTo(neighbor);
            if (dist < SMOOTHING_RADIUS) {
                density += neighbor.getMass() * smoothingKernel(dist);
            }
        }
    }
    // avoid zero density
    return std::max(density, EPSILON);
}

// Pressure using Tait (ideal gas-like) equation: p = k (rho - rho0)
double FluidSimulation::pressureFromDensity(double density) {
    // keep pressure non-negative (clamping helps stability)
    double p = GAS_CONSTANT * (density - REST_DENSITY);
    return std::max(p, 0.0);
}

// -------------------- Viscosity force computation --------------------

// Viscosity force on particle i (vector)
Vec2 FluidSimulation::viscosityOf(const Particle& pi, size_t iIndex, const std::vector<double>& densities) {
    Vec2 force(0.0, 0.0);
    for (size_t j = 0; j < particles.size(); ++j) {
        if (iIndex == j) continue;
        const Particle& pj = particles[j];

        double dist = pi.distanceTo(pj);
        if (dist < SMOOTHING_RADIUS && dist > 0.0) {
            double lap = viscosityLaplacian(dist);
            // velocity difference v_j - v_i
            Vec2 vel_diff(pj.getVx() - pi.getVx(), pj.getVy() - pi.getVy());
            // Typical SPH viscosity term: mu * m_j * (v_j - v_i) / rho_j * laplacian
            force += vel_diff * (VISCOSITY_CONSTANT * pj.getMass() * lap / densities[j]);
        }
    }
    return force;
}

// -------------------- Update --------------------

void FluidSimulation::update() {
    size_t N = particles.size();
    if (N == 0) return;

    // 1) Compute densities and pressures for all particles (store in vectors)
    std::vector<double> densities(N);
    std::vector<double> pressures(N);

    for (size_t i = 0; i < N; ++i) {
        densities[i] = densityOf(particles[i]);
        pressures[i] = pressureFromDensity(densities[i]);
    }

    // 2) For each particle compute pressure & viscosity forces (sum over neighbors)
    for (size_t i = 0; i < N; ++i) {
        Particle& pi = particles[i];

        Vec2 pressureForce(0.0, 0.0);
        Vec2 viscForce = viscosityOf(pi, i, densities);

        for (size_t j = 0; j < N; ++j) {
            if (i == j) continue;
            const Particle& pj = particles[j];

            double dist = pi.distanceTo(pj);
            if (dist < SMOOTHING_RADIUS && dist > 0.0) {
                // Gradient of spiky kernel (vector): (dW/dr) * (r_i - r_j)/r
                double dWdr = spikyKernelDerivative(dist); // already negative
                // direction from j to i:
                Vec2 dir( (pi.getX() - pj.getX()) / dist,
                          (pi.getY() - pj.getY()) / dist );

                // Pressure based acceleration: - sum_j m_j * (P_i + P_j)/(2 * rho_j) * gradW
                // Note: many variants divide by rho_i * rho_j; this variant is common and stable.
                double pressureTerm = (pressures[i] + pressures[j]) / (2.0 * densities[j] + EPSILON);
                // pressure force should be REPULSIVE; add the minus sign explicitly
                Vec2 contrib = dir * (-pj.getMass() * pressureTerm * dWdr);
                pressureForce += contrib;
            }
        }

        // Combine forces: pressureForce + viscForce + gravity
        // Gravity is applied as mass * gravity vector in applyForce; here we accumulate acceleration-like forces (mass accounted in applyForce).
        Vec2 totalForce = pressureForce + (gravity * pi.getMass()); //viscForce

        // Apply that force to the particle (assume Particle::applyForce(forceX, forceY, dt) exists)
        pi.applyForce(totalForce.x, totalForce.y, timeStep);

        // Optional: clamp velocity magnitude (prevents explosions if things get unstable)
        double maxVel = 5; // in world units per second; adjust as needed
        double vx = pi.getVx();
        double vy = pi.getVy();
        double speed2 = vx*vx + vy*vy;
        if (speed2 > maxVel*maxVel) {
            double speed = sqrt(speed2);
            vx = (vx / speed) * maxVel;
            vy = (vy / speed) * maxVel;
            pi.setVelocity(vx, vy);
        }

        // Integrate position
        pi.update(timeStep);

        // -------------------- boundaries --------------------
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
        if (dist < SMOOTHING_RADIUS) {
            density += neighbor.getMass() * smoothingKernel(dist);
        }
    }
    return std::max(density, EPSILON);
}

double FluidSimulation::getRestDensity() const {
    return REST_DENSITY;
}
