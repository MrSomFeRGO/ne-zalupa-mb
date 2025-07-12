//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);
    ~Camera() = default;

    // Движение
    void MoveForward(float delta);
    void MoveRight(float delta);
    void MoveUp(float delta);

    // Вращение
    void Rotate(float yaw, float pitch);

    // Обновление
    void SetAspectRatio(float aspectRatio);
    void SetPosition(const glm::vec3& position);

    // Getters
    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_projMatrix; }
    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetForward() const { return m_forward; }
    const glm::vec3& GetRight() const { return m_right; }
    const glm::vec3& GetUp() const { return m_up; }

    float GetYaw() const { return m_yaw; }
    float GetPitch() const { return m_pitch; }

private:
    void UpdateVectors();
    void UpdateMatrices();

    // Позиция и ориентация
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_worldUp;

    // Углы Эйлера
    float m_yaw;
    float m_pitch;

    // Параметры проекции
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // Матрицы
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projMatrix;
};