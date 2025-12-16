#include "ParticleRenderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>

static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
uniform float uPointSize;
uniform bool uUseVelocityColor;
uniform vec3 uDefaultColor;
out vec3 vColor;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = uPointSize;
    vColor = uUseVelocityColor ? aColor : uDefaultColor;
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

GLuint ParticleRenderer::compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

GLuint ParticleRenderer::linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
    }
    return prog;
}

ParticleRenderer::ParticleRenderer() {}

ParticleRenderer::~ParticleRenderer() {
    cleanup();
}

bool ParticleRenderer::init() {
    // Compile & link shaders
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Get uniform locations
    loc_uPointSize = glGetUniformLocation(shaderProgram, "uPointSize");
    loc_uDefaultColor = glGetUniformLocation(shaderProgram, "uDefaultColor");
    loc_uUseVelocityColor = glGetUniformLocation(shaderProgram, "uUseVelocityColor");

    // Create VAO + VBOs
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &colorVBO);

    glBindVertexArray(VAO);
    
    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // Color buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allow setting gl_PointSize from vertex shader
    glEnable(GL_PROGRAM_POINT_SIZE);

    return true;
}

void ParticleRenderer::cleanup() {
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (colorVBO) {
        glDeleteBuffers(1, &colorVBO);
        colorVBO = 0;
    }
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
}

void ParticleRenderer::draw(const std::vector<Particle>& particles, double maxVelocity) {
    if (particles.empty()) return;

    // Extract positions and colors from particles
    std::vector<float> positions;
    std::vector<float> colors;
    positions.reserve(particles.size() * 2);
    colors.reserve(particles.size() * 3);
    
    // Use maxVelocity from simulation for normalization (or calculate if not provided)
    float maxVel = static_cast<float>(maxVelocity);
    if (useVelocityColor && maxVel <= 0.0f) {
        // Fallback: calculate max velocity if not provided
        maxVel = 0.01f; // minimum threshold
        for (const auto& particle : particles) {
            float vx = static_cast<float>(particle.getVx());
            float vy = static_cast<float>(particle.getVy());
            float vel = std::sqrt(vx * vx + vy * vy);
            if (vel > maxVel) maxVel = vel;
        }
        if (maxVel < 0.01f) maxVel = 0.01f; // prevent division by zero
    }
    
    for (const auto& particle : particles) {
        // Position
        float x = particle.getX();
        float y = particle.getY();
        positions.push_back(x);
        positions.push_back(y);
        
        // Color based on velocity if enabled
        if (useVelocityColor && maxVel > 0.0f) {
            float vx = static_cast<float>(particle.getVx());
            float vy = static_cast<float>(particle.getVy());
            float vel = std::sqrt(vx * vx + vy * vy);
            float normalizedVel = std::min(1.0f, vel / maxVel);
            
            // Color gradient: blue (0 velocity) -> green -> yellow -> red (max velocity)
            float r, g, b;
            if (normalizedVel < 0.33f) {
                // Blue to green
                float t = normalizedVel / 0.33f;
                r = 0.0f;
                g = t;
                b = 1.0f - t;
            } else if (normalizedVel < 0.67f) {
                // Green to yellow
                float t = (normalizedVel - 0.33f) / 0.34f;
                r = t;
                g = 1.0f;
                b = 0.0f;
            } else {
                // Yellow to red
                float t = (normalizedVel - 0.67f) / 0.33f;
                r = 1.0f;
                g = 1.0f - t;
                b = 0.0f;
            }
            colors.push_back(r);
            colors.push_back(g);
            colors.push_back(b);
        } else {
            // Default light-blue color
            colors.push_back(0.2f);
            colors.push_back(0.6f);
            colors.push_back(1.0f);
        }
    }

    // Upload particle positions to GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);
    
    // Upload particle colors to GPU
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_DYNAMIC_DRAW);

    // draw
    glUseProgram(shaderProgram);

    // set uniforms
    float pointSize = 6.0f;
    glUniform1f(loc_uPointSize, pointSize);
    glUniform3f(loc_uDefaultColor, 0.2f, 0.6f, 1.0f);
    glUniform1i(loc_uUseVelocityColor, useVelocityColor ? 1 : 0);

    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(particles.size()));

    // cleanup bindings
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

