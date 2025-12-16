#include "UIControls.h"
#include "FluidSimulation.h"
#include "../external/imgui.h"
#include <cmath>

void UIControls::drawGui(FluidSimulation& sim) {
    ImGui::Begin("Simulation Controls");
    ImGui::Text("Gravity");
    // sync initial value if needed
    const Vec2& g = sim.getGravity();
    if (std::abs(uiGravityX - static_cast<float>(g.x)) > 1e-6f) {
        uiGravityX = static_cast<float>(g.x);
    }
    if (std::abs(uiGravityY - static_cast<float>(g.y)) > 1e-6f) {
        uiGravityY = static_cast<float>(g.y);
    }
    if (ImGui::SliderFloat("Gravity X", &uiGravityX, -2.0f, 2.0f, "%.6f")) {
        sim.setGravity(Vec2(static_cast<double>(uiGravityX), static_cast<double>(uiGravityY)));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Horizontal gravity component");
    }
    if (ImGui::SliderFloat("Gravity Y", &uiGravityY, -10.0f, 2.0f, "%.6f")) {
        sim.setGravity(Vec2(static_cast<double>(uiGravityX), static_cast<double>(uiGravityY)));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Vertical gravity component (negative = downward)");
    }

    ImGui::Separator();
    ImGui::Text("Smoothing Radius (h)");
    // sync UI value with simulation
    float simH = static_cast<float>(sim.getSmoothingRadius());
    if (std::abs(uiSmoothingRadius - simH) > 1e-6f) {
        uiSmoothingRadius = simH;
    }
    if (ImGui::SliderFloat("h", &uiSmoothingRadius, 0.005f, 0.5f, "%.5f")) {
        sim.setSmoothingRadius(static_cast<double>(uiSmoothingRadius));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Smoothing radius for SPH kernels (affects interaction range)");
    }

    ImGui::Separator();
    ImGui::Text("Pressure Multiplier");
    // sync UI value with simulation
    float simPM = static_cast<float>(sim.getPressureMultiplier());
    if (std::abs(uiPressureMultiplier - simPM) > 1e-6f) {
        uiPressureMultiplier = simPM;
    }
    if (ImGui::SliderFloat("Pressure Multiplier", &uiPressureMultiplier, 0.5f, 10.0f, "%.5f")) {
        sim.setPressureMultiplier(static_cast<double>(uiPressureMultiplier));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Multiplier for regular pressure force");
    }

    ImGui::Separator();
    ImGui::Text("Near Pressure Multiplier");
    // sync UI value with simulation
    float simNPM = static_cast<float>(sim.getNearPressureMultiplier());
    if (std::abs(uiNearPressureMultiplier - simNPM) > 1e-6f) {
        uiNearPressureMultiplier = simNPM;
    }
    if (ImGui::SliderFloat("Near Pressure Multiplier", &uiNearPressureMultiplier, 0.1f, 20.0f, "%.5f")) {
        sim.setNearPressureMultiplier(static_cast<double>(uiNearPressureMultiplier));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Multiplier for near pressure force (prevents particle clustering)");
    }

    ImGui::Separator();
    ImGui::Text("Viscosity Strength");
    // sync UI value with simulation
    float simVisc = static_cast<float>(sim.getViscosityStrength());
    if (std::abs(uiViscosityStrength - simVisc) > 1e-6f) {
        uiViscosityStrength = simVisc;
    }
    if (ImGui::SliderFloat("Viscosity Strength", &uiViscosityStrength, 0.0f, 1.0f, "%.5f")) {
        sim.setViscosityStrength(static_cast<double>(uiViscosityStrength));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Strength of viscosity force (smooths velocity differences between particles)");
    }

    ImGui::Separator();
    ImGui::Text("Max Velocity");
    float simMaxVel = static_cast<float>(sim.getMaxVelocity());
    if (std::abs(uiMaxVelocity - simMaxVel) > 1e-6f) {
        uiMaxVelocity = simMaxVel;
    }
    if (ImGui::SliderFloat("Max Velocity", &uiMaxVelocity, 0.1f, 50.0f, "%.2f")) {
        sim.setMaxVelocity(static_cast<double>(uiMaxVelocity));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Maximum velocity clamp (0 = no limit)");
    }

    ImGui::Separator();
    ImGui::Text("Time Step");
    // sync UI value with simulation
    float simTimeStep = sim.getTimeStep();
    if (std::abs(uiTimeStep - simTimeStep) > 1e-6f) {
        uiTimeStep = simTimeStep;
    }
    if (ImGui::SliderFloat("Time Step", &uiTimeStep, 0.001f, 0.02f, "%.6f")) {
        sim.setTimeStep(uiTimeStep);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Simulation time step (smaller = more stable but slower)");
    }

    ImGui::Separator();
    ImGui::Text("Damping");
    // sync UI value with simulation
    float simDamping = sim.getDamping();
    if (std::abs(uiDamping - simDamping) > 1e-6f) {
        uiDamping = simDamping;
    }
    if (ImGui::SliderFloat("Damping", &uiDamping, 0.0f, 1.0f, "%.3f")) {
        sim.setDamping(uiDamping);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("General velocity damping");
    }

    ImGui::Separator();
    ImGui::Text("Collision Damping");
    // sync UI value with simulation
    float simCollisionDamping = sim.getCollisionDamping();
    if (std::abs(uiCollisionDamping - simCollisionDamping) > 1e-6f) {
        uiCollisionDamping = simCollisionDamping;
    }
    if (ImGui::SliderFloat("Collision Damping", &uiCollisionDamping, 0.0f, 1.0f, "%.3f")) {
        sim.setCollisionDamping(uiCollisionDamping);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Velocity damping specifically for boundary collisions (1.0 = no damping, 0.0 = full damping)");
    }

    ImGui::Separator();
    ImGui::Text("Interaction");
    ImGui::SliderFloat("Interact Radius", &uiInteractRadius, 0.01f, 0.5f, "%.3f");
    ImGui::SliderFloat("Interact Strength", &uiInteractStrength, 0.0f, 20.0f, "%.2f");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Left click attracts, right click repels; higher = stronger force");
    }

    ImGui::Separator();
    ImGui::Text("Rest Density");
    // sync UI value with simulation
    float simRestDensity = static_cast<float>(sim.getRestDensity());
    if (std::abs(uiRestDensity - simRestDensity) > 1e-6f) {
        uiRestDensity = simRestDensity;
    }
    if (ImGui::SliderFloat("Rest Density", &uiRestDensity, 0.1f, 5.0f, "%.3f")) {
        sim.setRestDensity(static_cast<double>(uiRestDensity));
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Target density for pressure calculation");
    }

    ImGui::Separator();
    ImGui::Checkbox("Color by Velocity", &useVelocityColor);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Color particles based on their velocity magnitude (blue=slow, red=fast)");
    }
    
    ImGui::Separator();
    ImGui::Checkbox("Show Density Map", &showDensityMap);
    if (showDensityMap) {
        static const char* resItems[] = { "64", "128", "256" };
        if (ImGui::Combo("Density Res", &uiDensityResIndex, resItems, IM_ARRAYSIZE(resItems))) {
            // Resolution change handled by caller
        }
    }

    ImGui::Separator();
    ImGui::Text("Particle Spawn Settings");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Configure how particles are spawned when resetting");
    }
    ImGui::SliderInt("Particle Count", &uiParticleCount, 10, 2000, "%d");
    ImGui::SliderFloat("Spread X", &uiSpreadX, 0.1f, 4.0f, "%.2f");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Horizontal spread of particles from origin");
    }
    ImGui::SliderFloat("Spread Y", &uiSpreadY, 0.1f, 4.0f, "%.2f");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Vertical spread of particles from origin");
    }
    ImGui::SliderFloat("Origin X", &uiOriginX, -1.0f, 1.0f, "%.2f");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("X coordinate of spawn center");
    }
    ImGui::SliderFloat("Origin Y", &uiOriginY, -1.0f, 1.0f, "%.2f");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Y coordinate of spawn center");
    }

    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("Reset Simulation", ImVec2(-1, 0))) {
        resetRequested = true;
    }
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reset the simulation with new spawn settings");
    }
    ImGui::End();
}

