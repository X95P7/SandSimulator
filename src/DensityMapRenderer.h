#pragma once
#include <glad/glad.h>

class FluidSimulation; // forward declaration

// Handles rendering of density map background
class DensityMapRenderer {
private:
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint bgTexture = 0;
    GLuint quadProgram = 0;
    GLint loc_uTex = -1;
    
    int densityTexW = 256;
    int densityTexH = 256;
    bool enabled = false;
    
    static GLuint compileShader(GLenum type, const char* src);
    static GLuint linkProgram(GLuint vs, GLuint fs);
    
public:
    DensityMapRenderer();
    ~DensityMapRenderer();
    
    bool init();
    void cleanup();
    
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool getEnabled() const { return enabled; }
    
    void setResolution(int w, int h);
    int getWidth() const { return densityTexW; }
    int getHeight() const { return densityTexH; }
    
    void draw(const FluidSimulation& sim);
};

