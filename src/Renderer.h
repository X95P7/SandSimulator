#pragma once
#include <vector>
#include "Vec2.h"
#include "Particle.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class FluidSimulation; // forward declaration

class Renderer {
private:
    GLFWwindow* window;
    unsigned int VAO, VBO;
    int width, height;
    // density map resources
    unsigned int quadVAO, quadVBO;
    unsigned int bgTexture;
    bool showDensityMap = false;
    bool useVelocityColor = false;
    int densityTexW = 256;
    int densityTexH = 256;
    float uiGravityX = 0.0f;
    float uiGravityY = -10.0f;
    float uiSmoothingRadius = 0.16433f;
    float uiPressureMultiplier = 4.12456f;
    float uiNearPressureMultiplier = 0.93206f;
    float uiViscosityStrength = 0.0f;
    float uiMaxVelocity = 2.01f;
    float uiTimeStep = 0.005f;
    float uiDamping = 0.5f;
    float uiCollisionDamping = 0.0f;
    float uiRestDensity = 5.0f;
    // Interaction (mouse) settings
    float uiInteractRadius = 0.168f;
    float uiInteractStrength = 3.55f; // positive = attract, negative = repel
    int uiDensityResIndex = 1; // 0:64, 1:128, 2:256
    
    // Spawn settings
    int uiParticleCount = 300;
    float uiSpreadX = 1.6f;
    float uiSpreadY = 0.8f;
    float uiOriginX = 0.0f;
    float uiOriginY = 0.0f;
    
    // Reset flag
    bool resetRequested = false;
    // Interaction overlay state
    bool lastInteractActive = false;
    bool lastInteractRepel = false;
    float lastInteractNdcX = 0.0f;
    float lastInteractNdcY = 0.0f;
    float lastInteractRadius = 0.0f;
public:
    Renderer(int w, int h, const char* title);
    bool init();
    void beginFrame();
    void drawParticles(const std::vector<Particle>& particles, double maxVelocity = 0.0);
    void endFrame();
    bool shouldClose();
    void cleanup();
    // density map
    void setShowDensityMap(bool enabled);
    void drawDensityMap(const FluidSimulation& sim);
    // gui
    void drawGui(FluidSimulation& sim);
    // Mouse interaction: returns true if interaction is active, fills point/strength/radius
    bool getInteraction(Vec2& point, float& strength, float& radius);
    void drawInteractionOverlay();
    
    // Spawn settings access
    bool isResetRequested() const { return resetRequested; }
    void clearResetRequest() { resetRequested = false; }
    int getParticleCount() const { return uiParticleCount; }
    float getSpreadX() const { return uiSpreadX; }
    float getSpreadY() const { return uiSpreadY; }
    float getOriginX() const { return uiOriginX; }
    float getOriginY() const { return uiOriginY; }
};
