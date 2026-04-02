# HairSimu

A C++ course project for strand-based hair simulation with multi-model shading comparisons and optional interactive viewing.

## Final-enhanced features
- Particle-chain hair simulation with pinned roots and iterative constraints
- Multi-model shading comparisons:
  - Kajiya-Kay
  - Cook-Torrance-inspired
  - Color variation
  - Marschner approximation
  - Marschner parameterized model with R/TT/TRT components
  - Dual-scattering approximation
  - d'Eon energy-conserving approximation
- Optional root import from OBJ vertex lists to test the same model across shading methods
- Optional OpenGL viewer with real-time model switching

## Build
- cmake -S . -B build
- cmake --build build

If OpenGL and GLFW are available, viewer mode is enabled automatically. If not, the project still builds in analysis mode.

## Dependencies To Install
Required for the core C++ project:
- A C++17 compiler
- CMake 3.16+

Required for the optional interactive viewer:
- GLFW 3
- OpenGL development headers / framework

On macOS, OpenGL is provided by the system, and GLFW can be installed with Homebrew:
```bash
brew install glfw
```

If you have not installed the Apple command line tools yet, install them first:
```bash
xcode-select --install
```


## Launch viewer
-hair style 1
```bash
./build/HairSimu --viewer --hair-file references/models/straight.hair --max-strands 10000 --out output_hair_view
```
-hair style 2
```bash
./build/HairSimu --viewer --hair-file references/models/dark.hair --max-strands 10000 --out output_hair_view
```

Viewer hotkeys:
- 1: Kajiya-Kay
- 2: Cook-Torrance
- 3: Color-Variation
- 4: Marschner-Approx
- 5: Marschner-Param
- 6: Dual-Scattering-Approx
- 7: d'Eon-Energy-Conserving-Approx

- R: Stop/Start rotate
- "+"/"-": zoom 
## Model Sources
- Hair assets:
  - [references/models/straight.hair](references/models/straight.hair)
  - [references/models/dark.hair](references/models/dark.hair)
  - Source: the `resources/straight.hair` and `resources/dark.hair` assets from [sojinoh/hair-simulation-project-oh-pujol](https://github.com/sojinoh/hair-simulation-project-oh-pujol)
- Hair file parser:
  - [include/cyHairFile.h](include/cyHairFile.h)
  - Source: Cem Yuksel's cyHair file reader (`cyCodeBase`)
