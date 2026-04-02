#include "Experiment.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

#include "ShadingModels.h"

namespace {

std::string ToPpmColor(const Vec3& color) {
    const int r = static_cast<int>(std::max(0.0, std::min(255.0, color.x * 255.0)));
    const int g = static_cast<int>(std::max(0.0, std::min(255.0, color.y * 255.0)));
    const int b = static_cast<int>(std::max(0.0, std::min(255.0, color.z * 255.0)));
    return std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
}

}  // namespace

void RunComparisonExperiment(const HairSimulation& simulation, const ExperimentConfig& config) {
    std::filesystem::create_directories(config.outputDir);

    const auto& strands = simulation.GetStrands();
    const Vec3 viewDir = Normalize(Vec3{0.2, 0.5, 1.0});
    const Vec3 lightDir = Normalize(Vec3{-0.3, 0.8, 0.45});
    const Vec3 baseColor = {0.24, 0.16, 0.10};

    std::ofstream csv(config.outputDir + "/comparison.csv");
    csv << "strand_id,model,r,g,b,luminance,R,TT,TRT\n";

    std::map<std::string, double> modelLuminanceSum;
    std::map<std::string, int> modelCount;

    std::vector<std::vector<Vec3>> imageRows;
    std::vector<std::string> modelNames;

    for (size_t i = 0; i < strands.size(); ++i) {
        const auto& strand = strands[i];
        if (strand.particles.size() < 2) {
            continue;
        }

        const Vec3 tip = strand.particles.back().position;
        const Vec3 prev = strand.particles[strand.particles.size() - 2].position;
        const Vec3 tangent = Normalize(tip - prev);

        const auto samples = EvaluateAllShadingModels(static_cast<int>(i), tangent, viewDir, lightDir, baseColor, config.marschner);

        if (imageRows.empty()) {
            imageRows.resize(samples.size());
            for (const auto& s : samples) {
                modelNames.push_back(s.modelName);
            }
        }

        for (size_t m = 0; m < samples.size(); ++m) {
            const auto& sample = samples[m];
            csv << i << "," << sample.modelName << ","
                << std::fixed << std::setprecision(4)
                << sample.rgb.x << "," << sample.rgb.y << "," << sample.rgb.z << "," << sample.luminance << ","
                << sample.componentR << "," << sample.componentTT << "," << sample.componentTRT << "\n";

            modelLuminanceSum[sample.modelName] += sample.luminance;
            modelCount[sample.modelName] += 1;
            imageRows[m].push_back(sample.rgb);
        }
    }

    const int pixelScale = 4;
    const int width = static_cast<int>(strands.size()) * pixelScale;
    const int height = static_cast<int>(imageRows.size()) * 32;

    std::ofstream ppm(config.outputDir + "/comparison.ppm");
    ppm << "P3\n" << width << " " << height << "\n255\n";

    for (size_t row = 0; row < imageRows.size(); ++row) {
        for (int y = 0; y < 32; ++y) {
            for (const Vec3& c : imageRows[row]) {
                for (int x = 0; x < pixelScale; ++x) {
                    ppm << ToPpmColor(c) << " ";
                }
            }
            ppm << "\n";
        }
    }

    std::ofstream summary(config.outputDir + "/summary.txt");
    summary << "Hair shading comparison summary\n";
    summary << "Strands evaluated: " << strands.size() << "\n";
    summary << "Models:\n";

    for (const auto& name : modelNames) {
        const double avg = modelLuminanceSum[name] / std::max(1, modelCount[name]);
        summary << "- " << name << " average luminance: " << std::fixed << std::setprecision(4) << avg << "\n";
    }

    std::ofstream report(config.outputDir + "/report.md");
    report << "# Hair Shading Comparison Report\n\n";
    report << "## Setup\n";
    report << "- Strands: " << strands.size() << "\n";
    report << "- View dir: " << viewDir.x << ", " << viewDir.y << ", " << viewDir.z << "\n";
    report << "- Light dir: " << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << "\n";
    report << "- Base color: " << baseColor.x << ", " << baseColor.y << ", " << baseColor.z << "\n";
    report << "- Marschner params: betaR=" << config.marschner.betaR
           << ", betaTT=" << config.marschner.betaTT
           << ", betaTRT=" << config.marschner.betaTRT
           << ", eta=" << config.marschner.eta
           << ", melanin=" << config.marschner.melanin
           << ", cuticleTilt=" << config.marschner.cuticleTilt
           << ", absorptionStrength=" << config.marschner.absorptionStrength << "\n\n";

    report << "## Average Luminance\n";
    for (const auto& name : modelNames) {
        const double avg = modelLuminanceSum[name] / std::max(1, modelCount[name]);
        report << "- " << name << ": " << std::fixed << std::setprecision(4) << avg << "\n";
    }

    report << "\n## Output Files\n";
    report << "- comparison.csv\n";
    report << "- comparison.ppm\n";
    report << "- summary.txt\n";
    report << "- report.md\n";

    std::cout << "Wrote output files to: " << config.outputDir << "\n";
}
