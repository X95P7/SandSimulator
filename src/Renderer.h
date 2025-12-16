#pragma once
#include <vector>
#include "Vec2.h"
#include "Particle.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Forward declarations
class FluidSimulation;
class ParticleRenderer;
class DensityMapRenderer;
class UIControls;
class InteractionHandler;

// Main renderer class - orchestrates all rendering components
class Renderer {
private:
    GLFWwindow* window;
    int width, height;
    
    // Modular rendering components
    ParticleRenderer* particleRenderer = nullptr;
    DensityMapRenderer* densityMapRenderer = nullptr;
    UIControls* uiControls = nullptr;
    InteractionHandler* interactionHandler = nullptr;
    
public:
    Renderer(int w, int h, const char* title);
    ~Renderer();
    
    bool init();
    void beginFrame();
    void drawParticles(const std::vector<Particle>& particles, double maxVelocity = 0.0);
    void drawDensityMap(const FluidSimulation& sim);
    void endFrame();
    bool shouldClose();
    void cleanup();
    
    // GUI
    void drawGui(FluidSimulation& sim);
    
    // Mouse interaction
    bool getInteraction(Vec2& point, float& strength, float& radius);
    void drawInteractionOverlay();
    
    // Spawn settings access (delegated to UIControls)
    bool isResetRequested() const;
    void clearResetRequest();
    int getParticleCount() const;
    float getSpreadX() const;
    float getSpreadY() const;
    float getOriginX() const;
    float getOriginY() const;
};
