# Hair Shading Analysis Report

Source CSV: `output_final/comparison.csv`

## Summary
- Color-Variation: n=22, avg luminance=0.0634, std=0.0133, p10=0.0465, p90=0.0794
- Cook-Torrance: n=22, avg luminance=0.0239, std=0.0009, p10=0.0227, p90=0.0252
- Marschner-Approx: n=22, avg luminance=0.2255, std=0.0072, p10=0.2163, p90=0.2345
- Marschner-Param: n=22, avg luminance=0.0315, std=0.0001, p10=0.0314, p90=0.0315

## Suggested Interpretation
- Higher average luminance usually indicates stronger overall reflectance.
- Larger std/p90-p10 indicates a wider highlight distribution or more variation across strands.
- R/TT/TRT component plots are most meaningful for physically motivated models.
