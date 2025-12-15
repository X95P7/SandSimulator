#include <iostream>
#include <vector>
#include "FluidSimulation.h"
#include "Renderer.h"

using namespace std;

int main() {
    // Create simulation
    FluidSimulation sim(300); // 150 particles for example
    //FluidSimulation sim(15,15,0.06,Vec2(-0.5,-0.5)); // grid particles for example
    cout << "Test log" << endl;

    // Initialize renderer
    Renderer renderer(800, 600, "Fluid Simulation");
    if (!renderer.init()) {
        std::cerr << "Failed to initialize renderer\n";
        return -1;
    }
    renderer.setShowDensityMap(true);

    // Main loop
    while (!renderer.shouldClose()) {
        // Check if reset was requested
        if (renderer.isResetRequested()) {
            sim.resetParticles(
                renderer.getParticleCount(),
                renderer.getSpreadX(),
                renderer.getSpreadY(),
                renderer.getOriginX(),
                renderer.getOriginY()
            );
            renderer.clearResetRequest();
        }

        // Mouse interaction (left = attract, right = repel)
        Vec2 interactPoint;
        float interactStrength = 0.0f;
        float interactRadius = 0.0f;
        if (renderer.getInteraction(interactPoint, interactStrength, interactRadius)) {
            sim.applyInteraction(interactPoint, interactStrength, interactRadius);
        }
        
        sim.update();

        renderer.beginFrame();
        renderer.drawDensityMap(sim);
        renderer.drawParticles(sim.getPositions(), sim.getMaxVelocity());
        renderer.drawGui(sim);
        renderer.endFrame();
    }

    renderer.cleanup();
    return 0;
}
