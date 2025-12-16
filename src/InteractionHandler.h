#pragma once
#include "Vec2.h"
#include <GLFW/glfw3.h>

// Handles mouse interaction and overlay rendering
class InteractionHandler {
private:
    GLFWwindow* window = nullptr;
    int width = 0;
    int height = 0;
    
    float interactRadius = 0.168f;
    float interactStrength = 3.55f;
    
    // Overlay state
    bool lastInteractActive = false;
    bool lastInteractRepel = false;
    float lastInteractNdcX = 0.0f;
    float lastInteractNdcY = 0.0f;
    float lastInteractRadius = 0.0f;
    
public:
    InteractionHandler(GLFWwindow* win, int w, int h);
    
    void setWindowSize(int w, int h) { width = w; height = h; }
    void setInteractRadius(float r) { interactRadius = r; }
    void setInteractStrength(float s) { interactStrength = s; }
    
    // Returns true if interaction is active, fills point/strength/radius
    bool getInteraction(Vec2& point, float& strength, float& radius);
    
    // Draws interaction overlay
    void drawOverlay();
};

