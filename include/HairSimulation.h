#pragma once

#include <vector>

#include "MathTypes.h"

struct HairParticle {
    Vec3 position;
    Vec3 velocity;
    bool pinned = false;
};

struct HairStrand {
    std::vector<HairParticle> particles;
    double segmentLength = 0.03;
};

struct SimulationParams {
    double dt = 1.0 / 120.0;
    double damping = 0.04;
    double stiffness = 0.55;
    Vec3 gravity = {0.0, -9.81, 0.0};
    Vec3 wind = {0.0, 0.0, 0.0};
    int constraintIterations = 6;
};

class HairSimulation {
public:
    HairSimulation(int strandCount, int particlesPerStrand, double scalpRadius);
    HairSimulation(const std::vector<Vec3>& roots, int particlesPerStrand, double segmentLength = 0.03);
    HairSimulation(const std::vector<HairStrand>& initialStrands);

    void Step(const SimulationParams& params);
    const std::vector<HairStrand>& GetStrands() const;
    static std::vector<Vec3> GenerateScalpRoots(int strandCount, double scalpRadius);

private:
    std::vector<HairStrand> strands_;
};
