//
// Created by mrsomfergo on 13.07.2025.
//

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : m_position(0.0f, 20.0f, 0.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(-90.0f)  // Look along -Z axis
    , m_pitch(0.0f)
    , m_fov(fov)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane) {
    UpdateVectors();
    UpdateMatrices();
}

void Camera::MoveForward(float delta) {
    m_position += m_forward * delta;
    UpdateMatrices();
}

void Camera::MoveRight(float delta) {
    m_position += m_right * delta;
    UpdateMatrices();
}

void Camera::MoveUp(float delta) {
    m_position += m_worldUp * delta;
    UpdateMatrices();
}

void Camera::Rotate(float yaw, float pitch) {
    m_yaw += yaw;
    m_pitch += pitch;

    // Constrain pitch to avoid camera flip
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    UpdateVectors();
    UpdateMatrices();
}

void Camera::SetAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    UpdateMatrices();
}

void Camera::SetPosition(const glm::vec3& position) {
    m_position = position;
    UpdateMatrices();
}

void Camera::UpdateVectors() {
    // Calculate new forward vector
    glm::vec3 forward;
    forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    forward.y = sin(glm::radians(m_pitch));
    forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_forward = glm::normalize(forward);

    // Calculate right and up vectors
    m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void Camera::UpdateMatrices() {
    // Update view matrix
    m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, m_up);

    // Update projection matrix
    m_projMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}