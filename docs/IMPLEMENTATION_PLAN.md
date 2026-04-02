# Hair Simulation + Multi-Shader Comparison Plan

## Project Goal
Build a C++ hair simulation system that supports multiple shading models and produces comparable outputs from the same hair setup.

## Milestones
1. Baseline simulation
- Strand-based particles with pinned roots
- Gravity + wind + damping + segment-length constraints
- Deterministic simulation loop for reproducible comparisons

2. Baseline shading comparison (current repo stage)
- Cook-Torrance-inspired model
- Color variation model
- Marschner-style approximation
- CSV and image outputs for side-by-side evaluation

3. Interactive rendering (next)
- OpenGL rendering of strands
- Runtime model switching
- Camera/light controls

4. Evaluation + report
- Qualitative visual comparison (highlight shape, color richness, plausibility)
- Quantitative summary metrics per model
- Discussion of computational cost and limitations

## Risks and Mitigations
- Risk: Marschner full model complexity
- Mitigation: start from stable approximation, then refine based on paper equations

- Risk: performance with high strand counts
- Mitigation: start CPU prototype, then migrate shading to GLSL/GPU path

## Stretch Goals
- Add dual scattering approximation
- Add anisotropic roughness controls
- Add per-strand melanin-inspired color parametrization

## Key References
- Marschner et al. 2003 (hair scattering)
- d'Eon et al. 2011 (energy-conserving hair model)
- Existing course materials on BRDF, simulation, and rendering pipelines
