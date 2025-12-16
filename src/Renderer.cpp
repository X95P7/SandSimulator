// Renderer.cpp
#include "Renderer.h"
#include "FluidSimulation.h"
#include "ParticleRenderer.h"
#include "DensityMapRenderer.h"
#include "UIControls.h"
#include "InteractionHandler.h"
#include <iostream>
// imgui
#ifndef IMGUI_IMPL_OPENGL_LOADER_GLAD
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#endif
#include "../external/imgui.h"
#include "../external/backends/imgui_impl_glfw.h"
#include "../external/backends/imgui_impl_opengl3.h"

Renderer::Renderer(int w, int h, const char* title)
    : width(w), height(h), window(nullptr) {
    particleRenderer = new ParticleRenderer();
    densityMapRenderer = new DensityMapRenderer();
    uiControls = new UIControls();
}

Renderer::~Renderer() {
    cleanup();
    delete particleRenderer;
    delete densityMapRenderer;
    delete uiControls;
    delete interactionHandler;
}

bool Renderer::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Fluid Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    glViewport(0, 0, width, height);

    // Initialize modular components
    if (!particleRenderer->init()) {
        std::cerr << "Failed to initialize ParticleRenderer\n";
        return false;
    }
    
    if (!densityMapRenderer->init()) {
        std::cerr << "Failed to initialize DensityMapRenderer\n";
        return false;
    }
    
    interactionHandler = new InteractionHandler(window, width, height);

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Renderer::beginFrame() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // start imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Renderer::drawParticles(const std::vector<Particle>& particles, double maxVelocity) {
    particleRenderer->setUseVelocityColor(uiControls->getUseVelocityColor());
    particleRenderer->draw(particles, maxVelocity);
}

void Renderer::drawDensityMap(const FluidSimulation& sim) {
    densityMapRenderer->setEnabled(uiControls->getShowDensityMap());
    
    // Update resolution if needed
    int resIndex = uiControls->getDensityResIndex();
    int newW = 256, newH = 256;
    if (resIndex == 0) { newW = 64; newH = 64; }
    else if (resIndex == 1) { newW = 128; newH = 128; }
    else { newW = 256; newH = 256; }
    
    if (newW != densityMapRenderer->getWidth() || newH != densityMapRenderer->getHeight()) {
        densityMapRenderer->setResolution(newW, newH);
    }
    
    densityMapRenderer->draw(sim);
}

void Renderer::endFrame() {
    // overlay for interaction (mouse) before rendering ImGui
    drawInteractionOverlay();
    // render imgui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool Renderer::shouldClose() {
    return glfwWindowShouldClose(window);
}

void Renderer::cleanup() {
    // shutdown imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (particleRenderer) {
        particleRenderer->cleanup();
    }
    if (densityMapRenderer) {
        densityMapRenderer->cleanup();
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Renderer::drawGui(FluidSimulation& sim) {
    uiControls->drawGui(sim);
}

bool Renderer::getInteraction(Vec2& point, float& strength, float& radius) {
    if (!interactionHandler) return false;
    
    // Sync settings from UI
    interactionHandler->setInteractRadius(uiControls->getInteractRadius());
    interactionHandler->setInteractStrength(uiControls->getInteractStrength());
    
    return interactionHandler->getInteraction(point, strength, radius);
}

void Renderer::drawInteractionOverlay() {
    if (interactionHandler) {
        interactionHandler->drawOverlay();
    }
}

bool Renderer::isResetRequested() const {
    return uiControls ? uiControls->isResetRequested() : false;
}

void Renderer::clearResetRequest() {
    if (uiControls) {
        uiControls->clearResetRequest();
    }
}

int Renderer::getParticleCount() const {
    return uiControls ? uiControls->getParticleCount() : 300;
}

float Renderer::getSpreadX() const {
    return uiControls ? uiControls->getSpreadX() : 1.6f;
}

float Renderer::getSpreadY() const {
    return uiControls ? uiControls->getSpreadY() : 0.8f;
}

float Renderer::getOriginX() const {
    return uiControls ? uiControls->getOriginX() : 0.0f;
}

float Renderer::getOriginY() const {
    return uiControls ? uiControls->getOriginY() : 0.0f;
}
