#include "Viewer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#if defined(HAIRSIMU_ENABLE_VIEWER)
#include <GLFW/glfw3.h>

namespace {

ShadingModelType gCurrentModel = ShadingModelType::CookTorrance;
double gYawDegrees = 15.0;
double gPitchDegrees = 12.0;
double gDistance = 2.2;
bool gAutoRotate = true;

double Clamp(double v, double low, double high) {
    return std::max(low, std::min(high, v));
}

void SetPerspective(double fovYDegrees, double aspect, double zNear, double zFar) {
    const double f = 1.0 / std::tan(0.5 * fovYDegrees * 3.14159265358979323846 / 180.0);
    double matrix[16] = {
        f / aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (zFar + zNear) / (zNear - zFar), -1.0,
        0.0, 0.0, (2.0 * zFar * zNear) / (zNear - zFar), 0.0,
    };
    glLoadMatrixd(matrix);
}

void HandleKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_1) {
        gCurrentModel = ShadingModelType::CookTorrance;
        std::cout << "Switched to model: Cook-Torrance\n";
    } else if (key == GLFW_KEY_2) {
        gCurrentModel = ShadingModelType::ColorVariation;
        std::cout << "Switched to model: Color-Variation\n";
    } else if (key == GLFW_KEY_3) {
        gCurrentModel = ShadingModelType::MarschnerApprox;
        std::cout << "Switched to model: Marschner-Approx\n";
    } else if (key == GLFW_KEY_4) {
        gCurrentModel = ShadingModelType::MarschnerParametrized;
        std::cout << "Switched to model: Marschner-Param\n";
    } else if (key == GLFW_KEY_5) {
        gCurrentModel = ShadingModelType::KajiyaKay;
        std::cout << "Switched to model: Kajiya-Kay\n";
    } else if (key == GLFW_KEY_6) {
        gCurrentModel = ShadingModelType::DualScatteringApprox;
        std::cout << "Switched to model: Dual-Scattering-Approx\n";
    } else if (key == GLFW_KEY_7) {
        gCurrentModel = ShadingModelType::DEonEnergyConservingApprox;
        std::cout << "Switched to model: dEon-Energy-Conserving-Approx\n";
    } else if (key == GLFW_KEY_LEFT) {
        gYawDegrees -= 6.0;
    } else if (key == GLFW_KEY_RIGHT) {
        gYawDegrees += 6.0;
    } else if (key == GLFW_KEY_UP) {
        gPitchDegrees = Clamp(gPitchDegrees + 4.0, -75.0, 75.0);
    } else if (key == GLFW_KEY_DOWN) {
        gPitchDegrees = Clamp(gPitchDegrees - 4.0, -75.0, 75.0);
    } else if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        gDistance = Clamp(gDistance - 0.15, 0.9, 4.5);
    } else if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        gDistance = Clamp(gDistance + 0.15, 0.9, 4.5);
    } else if (key == GLFW_KEY_R) {
        gAutoRotate = !gAutoRotate;
        std::cout << "Auto-rotate: " << (gAutoRotate ? "ON" : "OFF") << "\n";
    }
}

}  // namespace

int RunViewer(
    const HairSimulation& simulation,
    const MarschnerParams& marschnerParams,
    const std::string& title) {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(1200, 840, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Window creation failed\n";
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, HandleKey);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(1.25f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto& strands = simulation.GetStrands();
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double minZ = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    double maxZ = std::numeric_limits<double>::lowest();
    Vec3 rootSum{0.0, 0.0, 0.0};
    for (const auto& strand : strands) {
        for (const auto& p : strand.particles) {
            minX = std::min(minX, p.position.x);
            minY = std::min(minY, p.position.y);
            minZ = std::min(minZ, p.position.z);
            maxX = std::max(maxX, p.position.x);
            maxY = std::max(maxY, p.position.y);
            maxZ = std::max(maxZ, p.position.z);
        }
        if (!strand.particles.empty()) {
            rootSum += strand.particles.front().position;
        }
    }

    if (minX >= maxX || minY >= maxY || minZ >= maxZ || strands.empty()) {
        minX = -1.0;
        maxX = 1.0;
        minY = -1.0;
        maxY = 1.0;
        minZ = -1.0;
        maxZ = 1.0;
    }

    const Vec3 sceneCenter = {(minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5};
    const Vec3 extent = {maxX - minX, maxY - minY, maxZ - minZ};
    const double sceneRadius = std::max({extent.x, extent.y, extent.z, 1e-6}) * 0.65;

    const Vec3 lightDir = Normalize(Vec3{-0.3, 0.8, 0.45});
    const Vec3 baseColor = {0.24, 0.16, 0.10};

    std::cout << "Viewer controls: 1-7 switch shader, Arrow keys rotate, +/- zoom, R auto-rotate, Esc exit\n";

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const double dt = now - lastTime;
        lastTime = now;
        if (gAutoRotate) {
            gYawDegrees += 18.0 * dt;
        }

        int w = 0;
        int h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.90f, 0.91f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        const double aspect = (h > 0) ? static_cast<double>(w) / h : 1.0;
        SetPerspective(45.0, aspect, 0.05, 20.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslated(0.0, -0.06, -gDistance - sceneRadius * 0.9);
        glRotated(gPitchDegrees, 1.0, 0.0, 0.0);
        glRotated(gYawDegrees, 0.0, 1.0, 0.0);
        glTranslated(-sceneCenter.x, -sceneCenter.y, -sceneCenter.z);

        for (size_t i = 0; i < strands.size(); ++i) {
            const auto& strand = strands[i];
            if (strand.particles.size() < 2) {
                continue;
            }

            const Vec3 tip = strand.particles.back().position;
            const double yawRad = gYawDegrees * 3.14159265358979323846 / 180.0;
            const double pitchRad = gPitchDegrees * 3.14159265358979323846 / 180.0;
            const Vec3 camDir = Normalize(Vec3{std::sin(yawRad) * std::cos(pitchRad), std::sin(pitchRad), std::cos(yawRad) * std::cos(pitchRad)});
            const Vec3 viewDir = Normalize(camDir * gDistance + (sceneCenter - tip));
            const Vec3 tangent = Normalize(strand.particles.back().position - strand.particles[strand.particles.size() - 2].position);
            const ShadingSample s = EvaluateShadingModel(gCurrentModel, static_cast<int>(i), tangent, viewDir, lightDir, baseColor, marschnerParams);
            glColor4d(s.rgb.x, s.rgb.y, s.rgb.z, 0.92);

            glBegin(GL_LINE_STRIP);
            for (const auto& p : strand.particles) {
                glVertex3d(p.position.x, p.position.y, p.position.z);
            }
            glEnd();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#else

int RunViewer(const HairSimulation&, const MarschnerParams&, const std::string&, const std::string&, double, const Vec3&) {
    std::cerr << "Viewer is not enabled. Reconfigure with HAIRSIMU_BUILD_VIEWER=ON and ensure OpenGL + GLFW are installed.\n";
    return 1;
}

#endif
