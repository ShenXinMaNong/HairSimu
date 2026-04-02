#pragma once

#include <string>

#include "HairSimulation.h"
#include "ShadingModels.h"

struct ExperimentConfig {
    int simulationSteps = 300;
    std::string outputDir = "output";
    MarschnerParams marschner;
};

void RunComparisonExperiment(const HairSimulation& simulation, const ExperimentConfig& config);
