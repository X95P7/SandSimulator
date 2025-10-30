#include <iostream>
#include <vector>
#include "FluidSimulation.h"
#include "Renderer.h"

using namespace std;

int main() {
    // Create simulation
    FluidSimulation sim(150); // 150 particles for example
    //FluidSimulation sim(10,10,0.1,Vec2(0.0,0.0)); // grid particles for example
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
        sim.update();

        renderer.beginFrame();
        renderer.drawDensityMap(sim);
        renderer.drawParticles(sim.getPositions());
        renderer.drawGui(sim);
        renderer.endFrame();
    }

    renderer.cleanup();
    return 0;
}
