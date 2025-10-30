// Renderer.cpp
#include "Renderer.h"
#include "FluidSimulation.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Simple shader sources
static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform float uPointSize;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = uPointSize;
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

// Fullscreen quad shader for density texture
static const char* quadVertexSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 vUV;
void main(){
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* quadFragmentSrc = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main(){
    vec3 col = texture(uTex, vUV).rgb;
    FragColor = vec4(col, 1.0);
}
)";

// helper: compile shader and print errors
static GLuint compileShader(GLenum type, const char* src) {
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

// helper: link program and print errors
static GLuint linkProgram(GLuint vs, GLuint fs) {
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

// keep shader program and uniform locations in cpp file scope
static GLuint shaderProgram = 0;
static GLint loc_uPointSize = -1;
static GLint loc_uColor = -1;
static GLuint quadProgram = 0;
static GLint loc_uTex = -1;

Renderer::Renderer(int w, int h, const char* title)
    : width(w), height(h), window(nullptr), VAO(0), VBO(0) {}

bool Renderer::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // core profile (we use modern OpenGL)
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

    // compile & link shaders
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // get uniform locations
    loc_uPointSize = glGetUniformLocation(shaderProgram, "uPointSize");
    loc_uColor = glGetUniformLocation(shaderProgram, "uColor");

    // compile quad shaders
    GLuint qvs = compileShader(GL_VERTEX_SHADER, quadVertexSrc);
    GLuint qfs = compileShader(GL_FRAGMENT_SHADER, quadFragmentSrc);
    quadProgram = linkProgram(qvs, qfs);
    glDeleteShader(qvs);
    glDeleteShader(qfs);
    loc_uTex = glGetUniformLocation(quadProgram, "uTex");

    // create VAO + VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // initially allocate zero bytes; we'll fill each frame with glBufferData
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // layout: location 0 -> vec2 (two floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // unbind to be safe
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // allow setting gl_PointSize from vertex shader
    glEnable(GL_PROGRAM_POINT_SIZE);

    // setup fullscreen quad (two triangles) with positions and UVs
    float quadVerts[] = {
        // pos      // uv
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // create density background texture
    glGenTextures(1, &bgTexture);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, densityTexW, densityTexH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Renderer::beginFrame() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::setShowDensityMap(bool enabled) {
    showDensityMap = enabled;
}

void Renderer::drawParticles(const std::vector<Particle>& particles) {
    if (particles.empty()) return;

    // Extract positions from particles and convert to OpenGL coordinates
    std::vector<float> positions;
    positions.reserve(particles.size() * 2);
    
    for (const auto& particle : particles) {
        // Convert from simulation coordinates (-1 to 1) to OpenGL coordinates
        float x = particle.getX();
        float y = particle.getY();
        
        // Normalize to OpenGL coordinate system (-1 to 1)
        positions.push_back(x);
        positions.push_back(y);
    }

    // Upload particle positions to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_DYNAMIC_DRAW);

    // draw
    glUseProgram(shaderProgram);

    // set uniforms (point size in pixels, color)
    float pointSize = 6.0f; // adjust as needed
    glUniform1f(loc_uPointSize, pointSize);
    // light-blue color
    glUniform3f(loc_uColor, 0.2f, 0.6f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(particles.size()));

    // cleanup bindings
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::endFrame() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool Renderer::shouldClose() {
    return glfwWindowShouldClose(window);
}

void Renderer::cleanup() {
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (quadProgram) {
        glDeleteProgram(quadProgram);
        quadProgram = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (bgTexture) {
        glDeleteTextures(1, &bgTexture);
        bgTexture = 0;
    }
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

// Generate and draw the density background. Expects simulation space in [-1,1] for both axes
void Renderer::drawDensityMap(const FluidSimulation& sim) {
    if (!showDensityMap) return;

    const size_t texelCount = static_cast<size_t>(densityTexW) * densityTexH;
    std::vector<double> rho(texelCount);
    double rhoMin = std::numeric_limits<double>::infinity();
    double rhoMax = 0.0;

    // Pass 1: sample density and compute min/max
    for (int j = 0; j < densityTexH; ++j) {
        float y = -1.0f + (2.0f * (j + 0.5f) / static_cast<float>(densityTexH));
        for (int i = 0; i < densityTexW; ++i) {
            float x = -1.0f + (2.0f * (i + 0.5f) / static_cast<float>(densityTexW));
            double d = sim.densityAt(x, y);
            size_t idx = static_cast<size_t>(j) * densityTexW + static_cast<size_t>(i);
            rho[idx] = d;
            if (d < rhoMin) rhoMin = d;
            if (d > rhoMax) rhoMax = d;
        }
    }

    // Choose green pivot within observed range; prefer rest density if it lies between min/max
    double rho0 = sim.getRestDensity();
    double rhoGreen = rho0;
    if (rhoGreen < rhoMin || rhoGreen > rhoMax) {
        rhoGreen = rhoMin + 0.35 * (rhoMax - rhoMin);
    }
    // Define hi scale so we see variation above green
    double rhoHigh = rhoGreen + 0.65 * (rhoMax - rhoGreen);
    if (rhoHigh <= rhoGreen) rhoHigh = rhoGreen + 1.0; // fallback

    // Optional gamma to emphasize falloff
    auto saturate = [](double v){ return std::max(0.0, std::min(1.0, v)); };
    const double gamma = 0.8;

    std::vector<unsigned char> pixels(texelCount * 3);
    for (int j = 0; j < densityTexH; ++j) {
        for (int i = 0; i < densityTexW; ++i) {
            size_t li = static_cast<size_t>(j) * densityTexW + static_cast<size_t>(i);
            double d = rho[li];
            float r = 0.0f, g = 0.0f, b = 0.0f;
            if (d <= rhoGreen) {
                double t = saturate((d - rhoMin) / std::max(1e-12, (rhoGreen - rhoMin)));
                t = std::pow(t, gamma);
                r = 0.0f;
                g = static_cast<float>(t);
                b = static_cast<float>(1.0 - t);
            } else {
                double t = saturate((d - rhoGreen) / std::max(1e-12, (rhoHigh - rhoGreen)));
                t = std::pow(t, gamma);
                r = static_cast<float>(t);
                g = static_cast<float>(1.0 - t);
                b = 0.0f;
            }
            size_t pi = li * 3;
            pixels[pi + 0] = static_cast<unsigned char>(std::round(std::clamp(r, 0.0f, 1.0f) * 255.0f));
            pixels[pi + 1] = static_cast<unsigned char>(std::round(std::clamp(g, 0.0f, 1.0f) * 255.0f));
            pixels[pi + 2] = static_cast<unsigned char>(std::round(std::clamp(b, 0.0f, 1.0f) * 255.0f));
        }
    }

    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, densityTexW, densityTexH, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(quadProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgTexture);
    glUniform1i(loc_uTex, 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
