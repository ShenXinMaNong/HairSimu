#pragma once

#include <string>
#include <vector>

#include "HairSimulation.h"
#include "MathTypes.h"

std::vector<Vec3> LoadRootsFromObj(const std::string& path, size_t maxRoots = 600);
std::vector<HairStrand> LoadStrandsFromHairFile(const std::string& path, size_t maxStrands = 1500);
