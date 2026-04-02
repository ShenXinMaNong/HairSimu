#pragma once

#include <string>
#include <vector>

#include "MathTypes.h"

enum class ShadingModelType {
    KajiyaKay,
    CookTorrance,
    ColorVariation,
    MarschnerApprox,
    MarschnerParametrized,
    DualScatteringApprox,
    DEonEnergyConservingApprox
};

struct MarschnerParams {
    double betaR = 0.18;
    double betaTT = 0.28;
    double betaTRT = 0.38;
    double eta = 1.55;
    double melanin = 0.35;
    double cuticleTilt = 2.0;
    double absorptionStrength = 0.65;
};

struct ShadingSample {
    std::string modelName;
    Vec3 rgb;
    double luminance = 0.0;
    double componentR = 0.0;
    double componentTT = 0.0;
    double componentTRT = 0.0;
};

std::vector<ShadingSample> EvaluateAllShadingModels(
    int strandId,
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& marschnerParams = {});

ShadingSample EvaluateShadingModel(
    ShadingModelType model,
    int strandId,
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& marschnerParams = {});
