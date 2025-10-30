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
    bool showDensityMap = true;
    int densityTexW = 256;
    int densityTexH = 256;
public:
    Renderer(int w, int h, const char* title);
    bool init();
    void beginFrame();
    void drawParticles(const std::vector<Particle>& particles);
    void endFrame();
    bool shouldClose();
    void cleanup();
    // density map
    void setShowDensityMap(bool enabled);
    void drawDensityMap(const FluidSimulation& sim);
};
