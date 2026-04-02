#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "Experiment.h"
#include "HairSimulation.h"
#include "HairModelLoader.h"
#include "Viewer.h"

namespace {

int ReadIntArg(char** begin, char** end, const std::string& key, int defaultValue) {
    for (char** it = begin; it != end; ++it) {
        if (std::string(*it) == key && (it + 1) != end) {
            return std::atoi(*(it + 1));
        }
    }
    return defaultValue;
}

std::string ReadStringArg(char** begin, char** end, const std::string& key, const std::string& defaultValue) {
    for (char** it = begin; it != end; ++it) {
        if (std::string(*it) == key && (it + 1) != end) {
            return *(it + 1);
        }
    }
    return defaultValue;
}

double ReadDoubleArg(char** begin, char** end, const std::string& key, double defaultValue) {
    for (char** it = begin; it != end; ++it) {
        if (std::string(*it) == key && (it + 1) != end) {
            return std::atof(*(it + 1));
        }
    }
    return defaultValue;
}

bool HasFlag(char** begin, char** end, const std::string& key) {
    for (char** it = begin; it != end; ++it) {
        if (std::string(*it) == key) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main(int argc, char** argv) {
    const int strandCount = ReadIntArg(argv, argv + argc, "--strands", 320);
    const int particlesPerStrand = ReadIntArg(argv, argv + argc, "--particles", 18);
    const int maxStrands = ReadIntArg(argv, argv + argc, "--max-strands", 1200);
    int steps = ReadIntArg(argv, argv + argc, "--steps", 300);
    const std::string outDir = ReadStringArg(argv, argv + argc, "--out", "output");
    const std::string rootsObj = ReadStringArg(argv, argv + argc, "--roots-obj", "");
    const std::string hairFile = ReadStringArg(argv, argv + argc, "--hair-file", "");
    const bool runViewer = HasFlag(argv, argv + argc, "--viewer");

    MarschnerParams marschner;
    marschner.betaR = ReadDoubleArg(argv, argv + argc, "--beta-r", marschner.betaR);
    marschner.betaTT = ReadDoubleArg(argv, argv + argc, "--beta-tt", marschner.betaTT);
    marschner.betaTRT = ReadDoubleArg(argv, argv + argc, "--beta-trt", marschner.betaTRT);
    marschner.eta = ReadDoubleArg(argv, argv + argc, "--eta", marschner.eta);
    marschner.melanin = ReadDoubleArg(argv, argv + argc, "--melanin", marschner.melanin);
    marschner.cuticleTilt = ReadDoubleArg(argv, argv + argc, "--cuticle-tilt", marschner.cuticleTilt);
    marschner.absorptionStrength = ReadDoubleArg(argv, argv + argc, "--absorption", marschner.absorptionStrength);

    std::cout << "Initializing hair simulation...\n";
    std::vector<Vec3> roots;
    std::vector<HairStrand> importedStrands;

    if (!hairFile.empty()) {
        importedStrands = LoadStrandsFromHairFile(hairFile, static_cast<size_t>(std::max(1, maxStrands)));
        if (importedStrands.empty()) {
            std::cerr << "Could not load strands from HAIR file: " << hairFile << "\n";
        } else {
            std::cout << "Loaded strands from HAIR file: " << importedStrands.size() << "\n";
            if (!HasFlag(argv, argv + argc, "--steps")) {
                steps = 0;
                std::cout << "No --steps supplied for imported hair. Keeping original style (steps=0).\n";
            }
        }
    }

    if (!rootsObj.empty()) {
        roots = LoadRootsFromObj(rootsObj);
        if (roots.empty()) {
            std::cerr << "Could not load roots from OBJ: " << rootsObj << "\n";
        }
    }

    HairSimulation sim = !importedStrands.empty()
        ? HairSimulation(importedStrands)
        : (roots.empty()
            ? HairSimulation(strandCount, particlesPerStrand, 0.32)
            : HairSimulation(roots, particlesPerStrand, 0.03));

    SimulationParams params;
    if (!importedStrands.empty()) {
        params.gravity = {0.0, -2.0, 0.0};
        params.wind = {0.0, 0.0, 0.0};
        params.stiffness = 0.85;
        params.damping = 0.08;
    }
    for (int i = 0; i < steps; ++i) {
        sim.Step(params);
    }

    ExperimentConfig config;
    config.simulationSteps = steps;
    config.outputDir = outDir;
    config.marschner = marschner;

    RunComparisonExperiment(sim, config);

    if (runViewer) {
        std::cout << "Launching viewer. Hotkeys: 1-7 switch shader, Arrow keys rotate, +/- zoom, R auto-rotate, Esc exit\n";
        RunViewer(sim, marschner, "HairSimu Viewer");
    }

    std::cout << "Done. Generated CSV/PPM/summary outputs for shading comparison.\n";
    return 0;
}
