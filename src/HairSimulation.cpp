#include "HairSimulation.h"

#include <algorithm>
#include <cmath>

namespace {

Vec3 RootPositionOnScalp(int strandIndex, int strandCount, double radius) {
    const double goldenAngle = 2.399963229728653;
    const double t = static_cast<double>(strandIndex) / std::max(1, strandCount - 1);
    const double r = radius * std::sqrt(t);
    const double theta = strandIndex * goldenAngle;

    const double x = r * std::cos(theta);
    const double z = r * std::sin(theta);
    const double y = std::sqrt(std::max(0.0, radius * radius - x * x - z * z));

    return {x, y, z};
}

}  // namespace

HairSimulation::HairSimulation(int strandCount, int particlesPerStrand, double scalpRadius)
    : HairSimulation(GenerateScalpRoots(strandCount, scalpRadius), particlesPerStrand, 0.03) {}

HairSimulation::HairSimulation(const std::vector<Vec3>& roots, int particlesPerStrand, double segmentLength) {
    strands_.reserve(roots.size());

    for (const Vec3& root : roots) {
        HairStrand strand;
        strand.segmentLength = segmentLength;
        strand.particles.reserve(particlesPerStrand);
        const Vec3 growthDirection = Normalize(Vec3{root.x * 0.35, 1.0, root.z * 0.35});

        for (int p = 0; p < particlesPerStrand; ++p) {
            HairParticle particle;
            particle.position = root + growthDirection * (strand.segmentLength * static_cast<double>(p));
            particle.velocity = {0.0, 0.0, 0.0};
            particle.pinned = (p == 0);
            strand.particles.push_back(particle);
        }

        strands_.push_back(std::move(strand));
    }
}

HairSimulation::HairSimulation(const std::vector<HairStrand>& initialStrands) : strands_(initialStrands) {
    for (HairStrand& strand : strands_) {
        if (strand.particles.empty()) {
            continue;
        }
        strand.particles[0].pinned = true;
        for (size_t i = 1; i < strand.particles.size(); ++i) {
            strand.particles[i].pinned = false;
        }
    }
}

void HairSimulation::Step(const SimulationParams& params) {
    for (HairStrand& strand : strands_) {
        for (HairParticle& particle : strand.particles) {
            if (particle.pinned) {
                particle.velocity = {0.0, 0.0, 0.0};
                continue;
            }

            const Vec3 acceleration = params.gravity + params.wind;
            particle.velocity += acceleration * params.dt;
            particle.velocity *= (1.0 - params.damping);
            particle.position += particle.velocity * params.dt;
        }

        for (int iter = 0; iter < params.constraintIterations; ++iter) {
            for (size_t i = 1; i < strand.particles.size(); ++i) {
                HairParticle& prev = strand.particles[i - 1];
                HairParticle& curr = strand.particles[i];

                Vec3 delta = curr.position - prev.position;
                const double len = Length(delta);
                if (len < 1e-8) {
                    continue;
                }

                const double diff = (len - strand.segmentLength) / len;
                const Vec3 correction = delta * (0.5 * params.stiffness * diff);

                if (!prev.pinned) {
                    prev.position += correction;
                }
                if (!curr.pinned) {
                    curr.position -= correction;
                }
            }
        }
    }
}

const std::vector<HairStrand>& HairSimulation::GetStrands() const {
    return strands_;
}

std::vector<Vec3> HairSimulation::GenerateScalpRoots(int strandCount, double scalpRadius) {
    std::vector<Vec3> roots;
    roots.reserve(std::max(0, strandCount));
    for (int i = 0; i < strandCount; ++i) {
        roots.push_back(RootPositionOnScalp(i, strandCount, scalpRadius));
    }
    return roots;
}
