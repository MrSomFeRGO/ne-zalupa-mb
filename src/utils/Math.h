//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <random>

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

    // Random number generation
    class Random {
    public:
        // Random float between 0 and 1
        static float Float() {
            static thread_local std::random_device rd;
            static thread_local std::mt19937 gen(rd());
            static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);
            return dis(gen);
        }

        // Random float between min and max
        static float Float(float min, float max) {
            static thread_local std::random_device rd;
            static thread_local std::mt19937 gen(rd());
            std::uniform_real_distribution<float> range_dis(min, max);
            return range_dis(gen);
        }

        // Random int between min and max (inclusive)
        static int Int(int min, int max) {
            static thread_local std::random_device rd;
            static thread_local std::mt19937 gen(rd());
            std::uniform_int_distribution<int> int_dis(min, max);
            return int_dis(gen);
        }

        // Seed the random generator
        static void Seed(uint32_t seed) {
            static thread_local std::mt19937 gen;
            gen.seed(seed);
        }
    };

    // Distance functions
    inline float Distance(const glm::vec3& a, const glm::vec3& b) {
        return glm::length(b - a);
    }

    inline float DistanceSquared(const glm::vec3& a, const glm::vec3& b) {
        glm::vec3 diff = b - a;
        return glm::dot(diff, diff);
    }

    inline float Distance2D(const glm::vec2& a, const glm::vec2& b) {
        return glm::length(b - a);
    }

    // Point in shapes
    inline bool PointInBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax) {
        return point.x >= boxMin.x && point.x <= boxMax.x &&
               point.y >= boxMin.y && point.y <= boxMax.y &&
               point.z >= boxMin.z && point.z <= boxMax.z;
    }

    inline bool PointInSphere(const glm::vec3& point, const glm::vec3& center, float radius) {
        return DistanceSquared(point, center) <= radius * radius;
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

    // Angle conversion
    inline float DegToRad(float degrees) {
        return degrees * 0.01745329251f; // PI / 180
    }

    inline float RadToDeg(float radians) {
        return radians * 57.2957795131f; // 180 / PI
    }

    // Map value from one range to another
    inline float Map(float value, float inMin, float inMax, float outMin, float outMax) {
        return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
    }

    // Noise utilities
    inline float Fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    inline float Noise1D(float x) {
        int i = static_cast<int>(std::floor(x));
        float f = x - i;
        float u = Fade(f);

        return Lerp(Random::Float(), Random::Float(), u);
    }

    // Vector utilities
    inline glm::vec3 Cross(const glm::vec3& a, const glm::vec3& b) {
        return glm::cross(a, b);
    }

    inline float Dot(const glm::vec3& a, const glm::vec3& b) {
        return glm::dot(a, b);
    }

    inline glm::vec3 Normalize(const glm::vec3& v) {
        return glm::normalize(v);
    }

    inline float Length(const glm::vec3& v) {
        return glm::length(v);
    }

    // Constants
    constexpr float PI = 3.14159265359f;
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float HALF_PI = 1.57079632679f;
    constexpr float INV_PI = 0.31830988618f;
    constexpr float DEG_TO_RAD = 0.01745329251f;
    constexpr float RAD_TO_DEG = 57.2957795131f;

} // namespace Math