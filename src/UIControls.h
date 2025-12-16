#pragma once
#include "Vec2.h"

class FluidSimulation; // forward declaration

// Manages all ImGui UI state and rendering
class UIControls {
private:
    // Simulation parameter UI state
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
    
    // Rendering options
    bool useVelocityColor = true;
    bool showDensityMap = false;
    int uiDensityResIndex = 1; // 0:64, 1:128, 2:256
    
    // Interaction settings
    float uiInteractRadius = 0.168f;
    float uiInteractStrength = 3.55f;
    
    // Spawn settings
    int uiParticleCount = 300;
    float uiSpreadX = 1.6f;
    float uiSpreadY = 0.8f;
    float uiOriginX = 0.0f;
    float uiOriginY = 0.0f;
    
    // Reset flag
    bool resetRequested = false;
    
public:
    void drawGui(FluidSimulation& sim);
    
    // Getters for UI state
    bool getUseVelocityColor() const { return useVelocityColor; }
    bool getShowDensityMap() const { return showDensityMap; }
    int getDensityResIndex() const { return uiDensityResIndex; }
    
    float getInteractRadius() const { return uiInteractRadius; }
    float getInteractStrength() const { return uiInteractStrength; }
    
    bool isResetRequested() const { return resetRequested; }
    void clearResetRequest() { resetRequested = false; }
    
    int getParticleCount() const { return uiParticleCount; }
    float getSpreadX() const { return uiSpreadX; }
    float getSpreadY() const { return uiSpreadY; }
    float getOriginX() const { return uiOriginX; }
    float getOriginY() const { return uiOriginY; }
};

