//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace Math {

    // Lerp functions
    template<typename T>
    inline T Lerp(T a, T b, float t) {
        return a + (b - a) * t;
    }

    inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t) {
        return a + (b - a) * t;
    }

    // Clamp
    template<typename T>
    inline T Clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }

    // Smooth step
    inline float SmoothStep(float edge0, float edge1, float x) {
        x = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3.0f - 2.0f * x);
    }

    // Random float between 0 and 1
    inline float Random() {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // Random float between min and max
    inline float Random(float min, float max) {
        return min + Random() * (max - min);
    }

    // Random int between min and max (inclusive)
    inline int RandomInt(int min, int max) {
        return min + rand() % (max - min + 1);
    }

    // Distance between two points
    inline float Distance(const glm::vec3& a, const glm::vec3& b) {
        return glm::length(b - a);
    }

    // Distance squared (faster than Distance when you don't need the exact value)
    inline float DistanceSquared(const glm::vec3& a, const glm::vec3& b) {
        glm::vec3 diff = b - a;
        return glm::dot(diff, diff);
    }

    // Check if a point is inside a box
    inline bool PointInBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax) {
        return point.x >= boxMin.x && point.x <= boxMax.x &&
               point.y >= boxMin.y && point.y <= boxMax.y &&
               point.z >= boxMin.z && point.z <= boxMax.z;
    }

    // Ray-box intersection
    inline bool RayBoxIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                  const glm::vec3& boxMin, const glm::vec3& boxMax,
                                  float& tMin, float& tMax) {
        glm::vec3 invDir = 1.0f / rayDir;
        glm::vec3 t1 = (boxMin - rayOrigin) * invDir;
        glm::vec3 t2 = (boxMax - rayOrigin) * invDir;

        glm::vec3 tMinVec = glm::min(t1, t2);
        glm::vec3 tMaxVec = glm::max(t1, t2);

        tMin = glm::max(glm::max(tMinVec.x, tMinVec.y), tMinVec.z);
        tMax = glm::min(glm::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

        return tMax >= tMin && tMax >= 0.0f;
    }

    // Convert degrees to radians
    inline float DegToRad(float degrees) {
        return degrees * 0.01745329251f; // PI / 180
    }

    // Convert radians to degrees
    inline float RadToDeg(float radians) {
        return radians * 57.2957795131f; // 180 / PI
    }

    // Map a value from one range to another
    inline float Map(float value, float inMin, float inMax, float outMin, float outMax) {
        return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
    }
}
