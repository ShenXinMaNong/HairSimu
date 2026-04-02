#include "ShadingModels.h"

#include <algorithm>
#include <cmath>

namespace {

double Clamp01(double v) {
    return std::max(0.0, std::min(1.0, v));
}

Vec3 ClampColor(const Vec3& c) {
    return {Clamp01(c.x), Clamp01(c.y), Clamp01(c.z)};
}

double Hash01(int id) {
    const double s = std::sin(static_cast<double>(id) * 12.9898 + 78.233) * 43758.5453;
    return s - std::floor(s);
}

double Luminance(const Vec3& c) {
    return 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z;
}

double SafeAsin(double v) {
    return std::asin(std::max(-1.0, std::min(1.0, v)));
}

ShadingSample KajiyaKaySample(const Vec3& tangent, const Vec3& viewDir, const Vec3& lightDir, const Vec3& baseColor) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    const double ndotl = std::max(0.0, Dot(t, l));
    const Vec3 h = Normalize(v + l);
    const double alignment = std::max(0.0, 1.0 - std::abs(Dot(h, t)));
    const double spec = std::pow(alignment, 28.0);
    const Vec3 color = baseColor * (0.28 + 0.72 * ndotl) + Vec3{spec * 0.55, spec * 0.50, spec * 0.45};

    const Vec3 clamped = ClampColor(color);
    return {"Kajiya-Kay", clamped, Luminance(clamped), spec, 0.0, 0.0};
}

ShadingSample CookTorranceSample(const Vec3& tangent, const Vec3& viewDir, const Vec3& lightDir, const Vec3& baseColor) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    Vec3 n = Normalize(Cross(t, Vec3{0.0, 0.0, 1.0}));
    if (Length(n) < 1e-5) {
        n = Normalize(Cross(t, Vec3{1.0, 0.0, 0.0}));
    }

    const Vec3 h = Normalize(v + l);
    const double ndotl = std::max(0.0, Dot(n, l));
    const double ndotv = std::max(0.0, Dot(n, v));
    const double ndoth = std::max(0.0, Dot(n, h));
    const double vdoth = std::max(0.0, Dot(v, h));

    const double roughness = 0.35;
    const double alpha = roughness * roughness;
    const double alpha2 = alpha * alpha;
    const double denomD = (ndoth * ndoth) * (alpha2 - 1.0) + 1.0;
    const double D = alpha2 / (3.14159265359 * denomD * denomD + 1e-8);

    const double k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    const double Gv = ndotv / (ndotv * (1.0 - k) + k + 1e-8);
    const double Gl = ndotl / (ndotl * (1.0 - k) + k + 1e-8);
    const double G = Gv * Gl;

    const Vec3 F0 = {0.04, 0.04, 0.04};
    const double F = F0.x + (1.0 - F0.x) * std::pow(1.0 - vdoth, 5.0);

    const double spec = (D * G * F) / (4.0 * ndotv * ndotl + 1e-8);
    const Vec3 diffuse = baseColor * (0.6 * ndotl);
    const Vec3 color = diffuse + Vec3{spec, spec, spec};

    const Vec3 clamped = ClampColor(color);
    return {"Cook-Torrance", clamped, Luminance(clamped), spec, 0.0, 0.0};
}

ShadingSample ColorVariationSample(int strandId, const Vec3& tangent, const Vec3& lightDir, const Vec3& baseColor) {
    const Vec3 t = Normalize(tangent);
    const Vec3 l = Normalize(lightDir);
    const double ndotl = std::max(0.0, Dot(t, l));

    const double jitter = (Hash01(strandId) - 0.5) * 0.25;
    const Vec3 variedBase = {
        Clamp01(baseColor.x + jitter * 0.8),
        Clamp01(baseColor.y + jitter * 0.6),
        Clamp01(baseColor.z + jitter * 0.25),
    };

    const Vec3 color = variedBase * (0.35 + 0.75 * ndotl);
    const Vec3 clamped = ClampColor(color);
    return {"Color-Variation", clamped, Luminance(clamped), 0.0, 0.0, 0.0};
}

ShadingSample MarschnerApproxSample(const Vec3& tangent, const Vec3& viewDir, const Vec3& lightDir, const Vec3& baseColor) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    const double sinThetaI = Dot(l, t);
    const double sinThetaO = Dot(v, t);
    const double thetaD = 0.5 * std::abs(SafeAsin(sinThetaI) - SafeAsin(sinThetaO));

    const double R = std::exp(-thetaD * thetaD / 0.045);
    const double TT = 0.45 * std::exp(-thetaD * thetaD / 0.09);
    const double TRT = 0.25 * std::exp(-thetaD * thetaD / 0.12);

    const Vec3 color = baseColor * (0.25 + TT + TRT) + Vec3{R, R * 0.9, R * 0.8};
    const Vec3 clamped = ClampColor(color);
    return {"Marschner-Approx", clamped, Luminance(clamped), R, TT, TRT};
}

ShadingSample MarschnerParamSample(
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& p) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    const double thetaI = SafeAsin(Dot(l, t));
    const double thetaO = SafeAsin(Dot(v, t));
    const double thetaH = 0.5 * (thetaI + thetaO);
    const double thetaD = 0.5 * (thetaI - thetaO);

    const double cosThetaD = std::max(0.01, std::cos(thetaD));
    const double eta = std::max(1.01, p.eta);
    const double f0 = ((eta - 1.0) * (eta - 1.0)) / ((eta + 1.0) * (eta + 1.0));
    const double fresnel = f0 + (1.0 - f0) * std::pow(1.0 - cosThetaD, 5.0);

    const double shift = p.cuticleTilt * 0.017453292519943295;
    const double mR = std::exp(-std::pow(thetaH - shift, 2.0) / std::max(1e-5, p.betaR * p.betaR));
    const double mTT = std::exp(-std::pow(thetaH + 0.25 * shift, 2.0) / std::max(1e-5, p.betaTT * p.betaTT));
    const double mTRT = std::exp(-std::pow(thetaH + shift, 2.0) / std::max(1e-5, p.betaTRT * p.betaTRT));

    const double absorption = std::exp(-p.absorptionStrength * p.melanin / std::max(0.1, cosThetaD));

    const double R = fresnel * mR;
    const double TT = (1.0 - fresnel) * mTT * absorption;
    const double TRT = fresnel * (1.0 - fresnel) * mTRT * std::sqrt(absorption);

    const Vec3 scattered = baseColor * (0.18 + 0.9 * TT + 0.6 * TRT) + Vec3{R, 0.86 * R, 0.78 * R};
    const Vec3 clamped = ClampColor(scattered);
    return {"Marschner-Param", clamped, Luminance(clamped), R, TT, TRT};
}

ShadingSample DualScatteringApproxSample(
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& p) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    const double thetaI = SafeAsin(Dot(l, t));
    const double thetaO = SafeAsin(Dot(v, t));
    const double thetaD = 0.5 * (thetaI - thetaO);
    const double thetaH = 0.5 * (thetaI + thetaO);

    const double primary = std::exp(-std::pow(thetaH - p.cuticleTilt * 0.017453292519943295, 2.0) / std::max(1e-5, p.betaR * p.betaR));
    const double secondary = std::exp(-std::pow(thetaH + 0.5 * p.cuticleTilt * 0.017453292519943295, 2.0) / std::max(1e-5, p.betaTT * p.betaTT));

    const double multipleScatter = 0.18 + 0.42 * (1.0 - std::exp(-p.absorptionStrength * p.melanin)) + 0.12 * std::abs(std::sin(thetaD));
    const double attenuation = std::exp(-0.8 * p.melanin * std::abs(thetaD));

    const Vec3 color = baseColor * (0.30 + 0.85 * primary + 0.55 * secondary + multipleScatter * attenuation);
    const Vec3 clamped = ClampColor(color);
    return {"Dual-Scattering-Approx", clamped, Luminance(clamped), primary, secondary, multipleScatter};
}

ShadingSample DEonEnergyConservingApproxSample(
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& p) {
    const Vec3 t = Normalize(tangent);
    const Vec3 v = Normalize(viewDir);
    const Vec3 l = Normalize(lightDir);

    const double thetaI = SafeAsin(Dot(l, t));
    const double thetaO = SafeAsin(Dot(v, t));
    const double thetaD = 0.5 * (thetaI - thetaO);
    const double thetaH = 0.5 * (thetaI + thetaO);

    const double eta = std::max(1.01, p.eta);
    const double f0 = ((eta - 1.0) * (eta - 1.0)) / ((eta + 1.0) * (eta + 1.0));
    const double fresnel = f0 + (1.0 - f0) * std::pow(1.0 - std::max(0.0, Dot(v, l)), 5.0);

    const double sigmaA = std::max(0.02, p.absorptionStrength * p.melanin);
    const double transmittance = std::exp(-sigmaA * (1.0 + 2.0 * std::abs(thetaD)));
    const double rough = std::max(1e-5, p.betaR);
    const double diffuseWidth = std::max(1e-5, p.betaTT);
    const double specR = std::exp(-std::pow(thetaH - p.cuticleTilt * 0.017453292519943295, 2.0) / (rough * rough));
    const double specTT = std::exp(-std::pow(thetaH + 0.35 * p.cuticleTilt * 0.017453292519943295, 2.0) / (diffuseWidth * diffuseWidth));

    const double energyBudget = Clamp01(1.0 - 0.35 * sigmaA);
    const double R = energyBudget * fresnel * specR;
    const double TT = energyBudget * (1.0 - fresnel) * transmittance * specTT;
    const double TRT = energyBudget * 0.42 * fresnel * (1.0 - transmittance) * std::exp(-std::abs(thetaD) * 1.8);

    const Vec3 tint = Vec3{
        std::pow(baseColor.x, 0.92),
        std::pow(baseColor.y, 0.95),
        std::pow(baseColor.z, 0.98),
    };

    const Vec3 color = tint * (0.14 + 0.95 * TT + 0.52 * TRT) + Vec3{R, 0.84 * R, 0.70 * R};
    const Vec3 clamped = ClampColor(color);
    return {"dEon-Energy-Conserving-Approx", clamped, Luminance(clamped), R, TT, TRT};
}

}  // namespace

ShadingSample EvaluateShadingModel(
    ShadingModelType model,
    int strandId,
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& marschnerParams) {
    switch (model) {
        case ShadingModelType::KajiyaKay:
            return KajiyaKaySample(tangent, viewDir, lightDir, baseColor);
        case ShadingModelType::CookTorrance:
            return CookTorranceSample(tangent, viewDir, lightDir, baseColor);
        case ShadingModelType::ColorVariation:
            return ColorVariationSample(strandId, tangent, lightDir, baseColor);
        case ShadingModelType::MarschnerApprox:
            return MarschnerApproxSample(tangent, viewDir, lightDir, baseColor);
        case ShadingModelType::MarschnerParametrized:
            return MarschnerParamSample(tangent, viewDir, lightDir, baseColor, marschnerParams);
        case ShadingModelType::DualScatteringApprox:
            return DualScatteringApproxSample(tangent, viewDir, lightDir, baseColor, marschnerParams);
        case ShadingModelType::DEonEnergyConservingApprox:
            return DEonEnergyConservingApproxSample(tangent, viewDir, lightDir, baseColor, marschnerParams);
    }
    return CookTorranceSample(tangent, viewDir, lightDir, baseColor);
}

std::vector<ShadingSample> EvaluateAllShadingModels(
    int strandId,
    const Vec3& tangent,
    const Vec3& viewDir,
    const Vec3& lightDir,
    const Vec3& baseColor,
    const MarschnerParams& marschnerParams) {
    std::vector<ShadingSample> samples;
    samples.reserve(7);

    samples.push_back(EvaluateShadingModel(ShadingModelType::KajiyaKay, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::CookTorrance, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::ColorVariation, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::MarschnerApprox, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::MarschnerParametrized, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::DualScatteringApprox, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));
    samples.push_back(EvaluateShadingModel(ShadingModelType::DEonEnergyConservingApprox, strandId, tangent, viewDir, lightDir, baseColor, marschnerParams));

    return samples;
}
