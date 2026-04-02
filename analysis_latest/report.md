# Hair Shading Analysis Report

Source CSV: `output_latest/comparison.csv`

## Summary
- Color-Variation: n=2000, avg luminance=0.0611, std=0.0162, p10=0.0390, p90=0.0826
- Cook-Torrance: n=2000, avg luminance=0.0354, std=0.0219, p10=0.0000, p90=0.0652
- Dual-Scattering-Approx: n=2000, avg luminance=0.1053, std=0.0258, p10=0.0982, p90=0.1068
- Kajiya-Kay: n=2000, avg luminance=0.0497, std=0.0156, p10=0.0483, p90=0.0483
- Marschner-Approx: n=2000, avg luminance=0.4845, std=0.3042, p10=0.1316, p90=0.9840
- Marschner-Param: n=2000, avg luminance=0.0369, std=0.0201, p10=0.0311, p90=0.0403
- dEon-Energy-Conserving-Approx: n=2000, avg luminance=0.0322, std=0.0186, p10=0.0270, p90=0.0354

## Suggested Interpretation
- Higher average luminance usually indicates stronger overall reflectance.
- Larger std/p90-p10 indicates a wider highlight distribution or more variation across strands.
- R/TT/TRT component plots are most meaningful for physically motivated models.
