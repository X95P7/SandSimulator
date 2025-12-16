## Fluid Simulation Term Project

This project is a 2D fluid / particle simulation written in modern C++ with OpenGL and Dear ImGui. It was built as a term project to explore SPH-style fluids, interactive visualization, and a modular rendering architecture.

### Features

- **SPH-inspired fluid simulation**
  - Configurable smoothing radius, pressure multipliers, viscosity strength, damping, and rest density
  - Velocity clamping for stability
  - Boundary collision handling with adjustable collision damping
- **Interactive controls (ImGui)**
  - Gravity (X/Y)
  - Smoothing radius and pressure params
  - Time step, damping, collision damping
  - Rest density and viscosity strength
  - Toggle: color particles by velocity
  - Toggle: density map background + adjustable resolution (64/128/256)
  - Particle spawn settings (count, spread X/Y, origin X/Y) + “Reset Simulation” button
- **Mouse interaction**
  - Left click: attract particles
  - Right click: repel particles
  - Adjustable interaction radius and strength
- **Rendering**
  - Points rendered via OpenGL with optional velocity-based color gradient (blue → green → yellow → red)
  - Optional density-map overlay rendered as a fullscreen texture
  - Dear ImGui-based UI

### Project Structure (key files)

- `src/main.cpp` – Application entry point and main loop
- `src/FluidSimulation.h/.cpp` – Fluid simulation core (particles, forces, integration, parameters)
- `src/SPHKernels.h/.cpp` – SPH kernel helper functions (Spiky/Poly6 variants)
- `src/Particle.h/.cpp` – Particle data and integration
- `src/Renderer.h/.cpp` – High-level renderer that wires everything together
- `src/ParticleRenderer.h/.cpp` – Renders particles and handles velocity coloring
- `src/DensityMapRenderer.h/.cpp` – Generates and renders the density map texture
- `src/UIControls.h/.cpp` – Owns and draws all ImGui UI/state
- `src/InteractionHandler.h/.cpp` – Mouse interaction + overlay rendering
- `src/Vec2.h` – Simple 2D vector math
- `src/glad.c`, `include/glad/…`, `include/GLFW/…`, `lib/…` – OpenGL loader and GLFW
- `external/` – Dear ImGui core and OpenGL/GLFW backends

### Building (Windows, VS Code tasks)

Requirements:
- A C++17-capable compiler (e.g. MinGW-w64 / g++)
- OpenGL-capable GPU and drivers
- GLFW 3 DLL (`glfw3.dll`) and import libraries in `lib/`

With VS Code:

- Press **Ctrl+Shift+B** and run the **`build main.exe`** task.  
  This executes (simplified):

```bash
g++ -std=c++17 \
  src/main.cpp \
  src/Renderer.cpp \
  src/FluidSimulation.cpp \
  src/Particle.cpp \
  src/SPHKernels.cpp \
  src/ParticleRenderer.cpp \
  src/DensityMapRenderer.cpp \
  src/UIControls.cpp \
  src/InteractionHandler.cpp \
  src/glad.c \
  external/imgui.cpp \
  external/imgui_draw.cpp \
  external/imgui_tables.cpp \
  external/imgui_widgets.cpp \
  external/imgui_demo.cpp \
  external/backends/imgui_impl_glfw.cpp \
  external/backends/imgui_impl_opengl3.cpp \
  -I src -I include -I external -I external/backends \
  -DIMGUI_IMPL_OPENGL_LOADER_GLAD \
  -L lib -lglfw3dll -lopengl32 -lgdi32 -limm32 -lshell32 -lole32 -loleaut32 -luuid \
  -o main.exe
```

Then run `main.exe` from the workspace root.

### Controls & Usage

- **Camera / view**: The simulation runs in normalized coordinates \([-1, 1]\) in both X and Y.
- **Mouse**
  - Left button: attract particles around the cursor
  - Right button: repel particles
- **ImGui window: “Simulation Controls”**
  - Adjust gravity, smoothing radius, pressure/near-pressure multipliers
  - Tune viscosity strength, damping, collision damping, time step
  - Enable “Color by Velocity” for a velocity heatmap
  - Enable “Show Density Map” and change its resolution
  - Configure particle spawn parameters and click **Reset Simulation** to respawn

### Notes / Future Improvements

- Spatial hashing helpers exist in `FluidSimulation` and can be used to optimize neighbor queries.
- Viscosity force is currently stubbed out in `calculateViscosity` and can be extended for richer flows.
- Additional boundaries or obstacles could be added by extending `resolveCollisions` or by introducing geometry objects.