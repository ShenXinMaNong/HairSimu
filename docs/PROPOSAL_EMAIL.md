Subject: GRAPHICS PROJECT PROPOSAL

Dear Professor Slocum,

Team Members:
- [Name 1], [USC Email 1]
- [Name 2], [USC Email 2]
- [Name 3], [USC Email 3]

We propose to build a C++ hair simulation and rendering system for strand-based hair, with a focus on comparing multiple hair shading techniques under the same geometric and lighting conditions. The system will simulate a controllable hair model (multiple strands with per-particle physics) and render/output visual and quantitative results for different shading models, including Cook-Torrance-inspired specular shading, a strand color-variation model, and a Marschner-style reflectance approximation. Our expected output is a reproducible comparison pipeline with images and statistics from the same hair scene.

This project is computer-graphics-related because it combines physically motivated motion simulation, geometric representation of strands, and light transport/reflection modeling for realistic appearance. It uses topics we learned in class, including modeling, animation/simulation, rendering equations, BRDF/reflectance ideas, and experimental evaluation. New topics we expect to learn include hair-specific reflectance models in greater depth (especially Marschner-family models), practical implementation details for stable strand constraints, and potentially GPU acceleration or shader-based implementations for higher visual quality.

Expected programming languages and middleware:
- C++17
- CMake
- OpenGL/GLSL (planned for interactive rendering phase)
- GLFW + GLAD (planned window/context setup)
- ImGui (planned for runtime controls)
- Optional: tinyobjloader or Assimp (for loading scalp/hair guide geometry)

Similar project and differentiation:
- Similar reference: https://github.com/sojinoh/hair-simulation-project-oh-pujol
- Our planned difference is a stronger emphasis on side-by-side shading analysis under a unified simulation setup, including controlled experiments and quantitative comparison outputs (e.g., per-model luminance/specular metrics) on the same hair model and camera/light settings.

References we plan to use:
- Marschner et al., “Light Scattering from Human Hair Fibers,” SIGGRAPH 2003.
- d'Eon et al., “An Energy-Conserving Hair Reflectance Model,” SIGGRAPH 2011.
- Kajiya and Kay, “Rendering Fur with Three Dimensional Textures,” SIGGRAPH 1989.
- Additional tutorials/blogs/videos on practical hair rendering and shading implementation as needed.

Thank you,
[Team Name]
