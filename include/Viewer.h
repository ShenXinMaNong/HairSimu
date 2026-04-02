#pragma once

#include <string>

#include "HairSimulation.h"
#include "ShadingModels.h"

int RunViewer(
    const HairSimulation& simulation,
    const MarschnerParams& marschnerParams,
    const std::string& title);
