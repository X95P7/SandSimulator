#pragma once
#include <vector>
#include "Particle.h"
#include <glad/glad.h>

// Handles rendering of particles with optional velocity-based coloring
class ParticleRenderer {
private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint colorVBO = 0;
    GLuint shaderProgram = 0;
    GLint loc_uPointSize = -1;
    GLint loc_uDefaultColor = -1;
    GLint loc_uUseVelocityColor = -1;
    
    bool useVelocityColor = true;
    
    static GLuint compileShader(GLenum type, const char* src);
    static GLuint linkProgram(GLuint vs, GLuint fs);
    
public:
    ParticleRenderer();
    ~ParticleRenderer();
    
    bool init();
    void cleanup();
    
    void setUseVelocityColor(bool enabled) { useVelocityColor = enabled; }
    bool getUseVelocityColor() const { return useVelocityColor; }
    
    void draw(const std::vector<Particle>& particles, double maxVelocity = 0.0);
};

