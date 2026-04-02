#include "HairModelLoader.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>

#include "cyHairFile.h"

namespace {

double EndpointCompactness(const std::vector<HairStrand>& strands, bool useFront) {
    if (strands.empty()) {
        return 0.0;
    }

    Vec3 center{0.0, 0.0, 0.0};
    int count = 0;
    for (const auto& strand : strands) {
        if (strand.particles.empty()) {
            continue;
        }
        center += useFront ? strand.particles.front().position : strand.particles.back().position;
        count += 1;
    }
    if (count == 0) {
        return 0.0;
    }
    center = center / static_cast<double>(count);

    double sum = 0.0;
    for (const auto& strand : strands) {
        if (strand.particles.empty()) {
            continue;
        }
        const Vec3 p = useFront ? strand.particles.front().position : strand.particles.back().position;
        sum += Length(p - center);
    }
    return sum / static_cast<double>(count);
}

double EndpointAverageY(const std::vector<HairStrand>& strands, bool useFront) {
    double sum = 0.0;
    int count = 0;
    for (const auto& strand : strands) {
        if (strand.particles.empty()) {
            continue;
        }
        sum += useFront ? strand.particles.front().position.y : strand.particles.back().position.y;
        count += 1;
    }
    return (count > 0) ? (sum / static_cast<double>(count)) : 0.0;
}

void ReverseIfNeeded(std::vector<HairStrand>& strands) {
    if (strands.empty()) {
        return;
    }

    const double frontCompactness = EndpointCompactness(strands, true);
    const double backCompactness = EndpointCompactness(strands, false);
    const double frontY = EndpointAverageY(strands, true);
    const double backY = EndpointAverageY(strands, false);

    const bool compactnessSuggestsReverse = backCompactness < frontCompactness * 0.95;
    const bool heightSuggestsReverse = frontY < backY;
    const bool shouldReverse = compactnessSuggestsReverse || heightSuggestsReverse;

    if (!shouldReverse) {
        return;
    }

    for (auto& strand : strands) {
        std::reverse(strand.particles.begin(), strand.particles.end());
        for (size_t i = 0; i < strand.particles.size(); ++i) {
            strand.particles[i].pinned = (i == 0);
        }
    }
}

Vec3 RotateVectorRodrigues(const Vec3& v, const Vec3& axis, double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    return v * c + Cross(axis, v) * s + axis * (Dot(axis, v) * (1.0 - c));
}

void UprightAlignStrands(std::vector<HairStrand>& strands) {
    if (strands.empty()) {
        return;
    }

    Vec3 rootAvg{0.0, 0.0, 0.0};
    Vec3 tipAvg{0.0, 0.0, 0.0};
    int valid = 0;

    for (const auto& strand : strands) {
        if (strand.particles.size() < 2) {
            continue;
        }
        rootAvg += strand.particles.front().position;
        tipAvg += strand.particles.back().position;
        valid += 1;
    }
    if (valid == 0) {
        return;
    }

    rootAvg = rootAvg / static_cast<double>(valid);
    tipAvg = tipAvg / static_cast<double>(valid);

    Vec3 growthDir = Normalize(tipAvg - rootAvg);
    const Vec3 targetDir = {0.0, -1.0, 0.0};

    const double d = std::max(-1.0, std::min(1.0, Dot(growthDir, targetDir)));
    Vec3 axis = Cross(growthDir, targetDir);
    double axisLen = Length(axis);

    if (axisLen > 1e-7) {
        axis = axis / axisLen;
        const double angle = std::acos(d);
        for (auto& strand : strands) {
            for (auto& particle : strand.particles) {
                particle.position = RotateVectorRodrigues(particle.position, axis, angle);
            }
        }
    } else if (d < 0.0) {
        // 180-degree flip around X when directions are opposite.
        for (auto& strand : strands) {
            for (auto& particle : strand.particles) {
                particle.position.y = -particle.position.y;
                particle.position.z = -particle.position.z;
            }
        }
    }

    // Recenter horizontally and place roots near the top area for a clean standalone hairstyle view.
    Vec3 rootsCenter{0.0, 0.0, 0.0};
    double maxRootY = std::numeric_limits<double>::lowest();
    int rootCount = 0;
    for (const auto& strand : strands) {
        if (strand.particles.empty()) {
            continue;
        }
        const Vec3 root = strand.particles.front().position;
        rootsCenter += root;
        maxRootY = std::max(maxRootY, root.y);
        rootCount += 1;
    }
    if (rootCount == 0) {
        return;
    }

    rootsCenter = rootsCenter / static_cast<double>(rootCount);
    const Vec3 shift = {-rootsCenter.x, 0.35 - maxRootY, -rootsCenter.z};
    for (auto& strand : strands) {
        for (auto& particle : strand.particles) {
            particle.position += shift;
        }
    }
}

}  // namespace

std::vector<Vec3> LoadRootsFromObj(const std::string& path, size_t maxRoots) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return {};
    }

    std::vector<Vec3> vertices;
    vertices.reserve(maxRoots);

    std::string line;
    while (std::getline(in, line)) {
        if (line.size() < 3 || line[0] != 'v' || line[1] != ' ') {
            continue;
        }

        std::istringstream iss(line.substr(2));
        Vec3 v;
        if (!(iss >> v.x >> v.y >> v.z)) {
            continue;
        }
        vertices.push_back(v);
        if (vertices.size() >= maxRoots) {
            break;
        }
    }

    if (vertices.empty()) {
        return {};
    }

    Vec3 minV = vertices.front();
    Vec3 maxV = vertices.front();
    for (const Vec3& v : vertices) {
        minV.x = std::min(minV.x, v.x);
        minV.y = std::min(minV.y, v.y);
        minV.z = std::min(minV.z, v.z);
        maxV.x = std::max(maxV.x, v.x);
        maxV.y = std::max(maxV.y, v.y);
        maxV.z = std::max(maxV.z, v.z);
    }

    const Vec3 center = (minV + maxV) * 0.5;
    const Vec3 extent = maxV - minV;
    const double maxExtent = std::max({extent.x, extent.y, extent.z, 1e-6});
    const double scale = 0.6 / maxExtent;

    std::vector<Vec3> roots;
    roots.reserve(vertices.size());
    for (Vec3 v : vertices) {
        v -= center;
        v *= scale;
        roots.push_back(v);
    }

    return roots;
}

std::vector<HairStrand> LoadStrandsFromHairFile(const std::string& path, size_t maxStrands) {
    cy::HairFile file;
    const int loadResult = file.LoadFromFile(path.c_str());
    if (loadResult <= 0) {
        return {};
    }

    const auto& header = file.GetHeader();
    const unsigned int totalHairs = header.hair_count;
    const unsigned int totalPoints = header.point_count;
    const unsigned short* segments = file.GetSegmentsArray();
    const float* points = file.GetPointsArray();

    if (totalHairs == 0 || totalPoints == 0 || points == nullptr) {
        return {};
    }

    const size_t keepCount = std::min(maxStrands, static_cast<size_t>(totalHairs));
    const size_t stride = std::max<size_t>(1, static_cast<size_t>(totalHairs) / keepCount);

    std::vector<HairStrand> strands;
    strands.reserve(keepCount);

    unsigned int pointOffset = 0;
    Vec3 minV{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    Vec3 maxV{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};

    for (unsigned int hairIndex = 0; hairIndex < totalHairs; ++hairIndex) {
        const unsigned int segmentCount = (segments != nullptr) ? segments[hairIndex] : header.d_segments;
        const unsigned int pointCount = segmentCount + 1;
        if (pointOffset + pointCount > totalPoints) {
            break;
        }

        const bool keep = (hairIndex % stride == 0) && (strands.size() < keepCount);
        if (keep) {
            HairStrand strand;
            strand.particles.reserve(pointCount);

            double lengthAccum = 0.0;
            int lengthCount = 0;
            Vec3 prev{};

            for (unsigned int p = 0; p < pointCount; ++p) {
                const unsigned int idx = pointOffset + p;
                Vec3 pos{points[idx * 3 + 0], points[idx * 3 + 1], points[idx * 3 + 2]};

                minV.x = std::min(minV.x, pos.x);
                minV.y = std::min(minV.y, pos.y);
                minV.z = std::min(minV.z, pos.z);
                maxV.x = std::max(maxV.x, pos.x);
                maxV.y = std::max(maxV.y, pos.y);
                maxV.z = std::max(maxV.z, pos.z);

                HairParticle particle;
                particle.position = pos;
                particle.velocity = {0.0, 0.0, 0.0};
                particle.pinned = (p == 0);
                strand.particles.push_back(particle);

                if (p > 0) {
                    lengthAccum += Length(pos - prev);
                    lengthCount += 1;
                }
                prev = pos;
            }

            if (lengthCount > 0) {
                strand.segmentLength = std::max(0.002, lengthAccum / static_cast<double>(lengthCount));
            }
            strands.push_back(std::move(strand));
        }

        pointOffset += pointCount;
    }

    if (strands.empty()) {
        return {};
    }

    const Vec3 center = (minV + maxV) * 0.5;
    const Vec3 extent = maxV - minV;
    const double maxExtent = std::max({extent.x, extent.y, extent.z, 1e-6});
    const double scale = 1.0 / maxExtent;

    for (HairStrand& strand : strands) {
        for (HairParticle& particle : strand.particles) {
            particle.position -= center;
            particle.position *= scale;
            particle.position.y += 0.2;
        }
        strand.segmentLength *= scale;
    }

    ReverseIfNeeded(strands);
    UprightAlignStrands(strands);

    return strands;
}
