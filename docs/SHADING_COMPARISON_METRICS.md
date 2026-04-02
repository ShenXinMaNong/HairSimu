# Hair Shading Comparison Metrics

These are practical metrics you can use to compare hair shading models in a report or presentation.

## 1. Average Luminance
Measures how bright a model is overall.

- Useful for comparing general reflectance strength.
- A higher value usually means stronger highlight energy.

## 2. Luminance Standard Deviation
Measures how much intensity varies across strands.

- Useful for seeing whether a model creates more local contrast.
- Larger values often correspond to more noticeable highlight variation.

## 3. Luminance Range and Percentiles
Use min, max, p10, and p90 to describe the spread of brightness.

- p90 - p10 is a simple contrast score.
- More spread generally means a more dramatic or less uniform appearance.

## 4. Average RGB
Shows whether the model introduces color shifts.

- Useful for models with pigment variation or wavelength-dependent effects.
- Helps explain whether a model tends toward warmer or cooler appearance.

## 5. Scattering Component Energy
For physically motivated models, look at component averages like R, TT, and TRT.

- R: surface/specular reflection
- TT: transmission through the fiber
- TRT: internal reflection / deeper scattering contribution

These are useful when discussing Marschner-family or d'Eon-style models.

## 6. Suggested Figure Set
For a clean report, generate these figures:

- Average luminance bar chart
- Luminance boxplot
- Average RGB stacked bar chart
- Scattering component energy stacked bar chart

## 7. Good Presentation Angle
A strong comparison story is:

1. Simple empirical model
2. Microfacet baseline
3. Physically motivated scattering
4. More advanced multi-lobe or energy-conserving model

That makes the progression easy to explain to the instructor.
