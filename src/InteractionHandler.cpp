#include "InteractionHandler.h"
#include "../external/imgui.h"
#include <algorithm>

InteractionHandler::InteractionHandler(GLFWwindow* win, int w, int h)
    : window(win), width(w), height(h) {}

bool InteractionHandler::getInteraction(Vec2& point, float& strength, float& radius) {
    // Check mouse buttons
    bool left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (!left && !right) {
        lastInteractActive = false;
        return false;
    }

    // Get cursor position in window coordinates
    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);
    // Convert to simulation coords (-1..1)
    float ndcX = static_cast<float>((cx / width) * 2.0 - 1.0);
    float ndcY = static_cast<float>(1.0 - (cy / height) * 2.0); // invert Y

    point = Vec2(ndcX, ndcY);
    radius = interactRadius;
    strength = left ? interactStrength : -interactStrength; // left attracts, right repels

    // store for overlay
    lastInteractActive = true;
    lastInteractRepel = right;
    lastInteractNdcX = ndcX;
    lastInteractNdcY = ndcY;
    lastInteractRadius = interactRadius;
    return true;
}

void InteractionHandler::drawOverlay() {
    if (!lastInteractActive) return;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    float sx = (lastInteractNdcX * 0.5f + 0.5f) * static_cast<float>(width);
    float sy = (1.0f - (lastInteractNdcY * 0.5f + 0.5f)) * static_cast<float>(height);
    float sMin = static_cast<float>(std::min(width, height));
    float sr = lastInteractRadius * 0.5f * sMin;
    ImU32 col = ImColor(lastInteractRepel ? ImVec4(0.9f, 0.2f, 0.2f, 0.9f)
                                          : ImVec4(0.2f, 0.9f, 0.2f, 0.9f));
    dl->AddCircle(ImVec2(sx, sy), sr, col, 64, 2.0f);
    dl->AddCircleFilled(ImVec2(sx, sy), 4.0f, col);
}

