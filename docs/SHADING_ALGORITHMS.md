# Hair Shading Algorithm Classification

## 1. Empirical / Classic Real-Time Models
These are simpler, older, and often used as baselines.

- Kajiya-Kay: classic tangent-based strand shading with an anisotropic highlight.
- Color Variation: artistic model that varies the base strand color slightly per strand.

## 2. Microfacet / BRDF-Based Approximations
These use reflection models inspired by surface BRDF theory.

- Cook-Torrance: microfacet-inspired specular model, useful as a general baseline.
- Anisotropic microfacet hair variants can also fit here.

## 3. Physically Motivated Hair Scattering Models
These are closer to the actual light transport inside hair fibers.

- Marschner Approximation: simplified R/TT/TRT-style lobes.
- Marschner Parameterized: tunable version with explicit lobe control.
- Dual-Scattering Approximation: multiple-scattering style approximation that is closer to dense hair appearance.
- d'Eon Energy-Conserving Approximation: a more paper-like lobe-balanced model that emphasizes energy conservation and pigment-based attenuation.

## 4. How This Project Uses Them
Current project output includes:
- Kajiya-Kay
- Cook-Torrance
- Color Variation
- Marschner Approximation
- Marschner Parameterized
- Dual-Scattering Approximation
- d'Eon Energy-Conserving Approximation

This gives a clearer taxonomy than the older reference project, which mainly shows a smaller subset of model families.

## 5. Suggested Presentation Order
For a class demo, show them in this order:
1. Kajiya-Kay
2. Cook-Torrance
3. Color Variation
4. Marschner Approximation
5. Marschner Parameterized
6. Dual-Scattering Approximation
7. d'Eon Energy-Conserving Approximation

That sequence moves from older/simple to more physically grounded models.
