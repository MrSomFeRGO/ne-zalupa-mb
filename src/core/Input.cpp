//
// Created by mrsomfergo on 13.07.2025.
//

#include "Input.h"
#include <iostream>

Input::Input() {
}

void Input::SetKeyDown(SDL_Scancode key) {
    m_currentKeys[key] = true;
}

void Input::SetKeyUp(SDL_Scancode key) {
    m_currentKeys[key] = false;
}

bool Input::IsKeyDown(SDL_Scancode key) const {
    auto it = m_currentKeys.find(key);
    return it != m_currentKeys.end() && it->second;
}

bool Input::IsKeyPressed(SDL_Scancode key) const {
    auto curr = m_currentKeys.find(key);
    auto prev = m_previousKeys.find(key);

    bool currentDown = (curr != m_currentKeys.end() && curr->second);
    bool previousDown = (prev != m_previousKeys.end() && prev->second);

    return currentDown && !previousDown;
}

bool Input::IsKeyReleased(SDL_Scancode key) const {
    auto curr = m_currentKeys.find(key);
    auto prev = m_previousKeys.find(key);

    bool currentDown = (curr != m_currentKeys.end() && curr->second);
    bool previousDown = (prev != m_previousKeys.end() && prev->second);

    return !currentDown && previousDown;
}

void Input::SetMouseButtonDown(uint8_t button) {
    m_currentMouseButtons[button] = true;
}

void Input::SetMouseButtonUp(uint8_t button) {
    m_currentMouseButtons[button] = false;
}

bool Input::IsMouseButtonDown(uint8_t button) const {
    auto it = m_currentMouseButtons.find(button);
    return it != m_currentMouseButtons.end() && it->second;
}

bool Input::IsMouseButtonPressed(uint8_t button) const {
    auto curr = m_currentMouseButtons.find(button);
    auto prev = m_previousMouseButtons.find(button);

    bool currentDown = (curr != m_currentMouseButtons.end() && curr->second);
    bool previousDown = (prev != m_previousMouseButtons.end() && prev->second);

    return currentDown && !previousDown;
}

bool Input::IsMouseButtonReleased(uint8_t button) const {
    auto curr = m_currentMouseButtons.find(button);
    auto prev = m_previousMouseButtons.find(button);

    bool currentDown = (curr != m_currentMouseButtons.end() && curr->second);
    bool previousDown = (prev != m_previousMouseButtons.end() && prev->second);

    return !currentDown && previousDown;
}

void Input::SetMousePosition(int x, int y) {
    m_mouseX = x;
    m_mouseY = y;
}

void Input::SetMouseDelta(int dx, int dy) {
    m_mouseDeltaX += static_cast<float>(dx);
    m_mouseDeltaY += static_cast<float>(dy);
}

void Input::GetMousePosition(int& x, int& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

void Input::GetMouseDelta(float& dx, float& dy) const {
    dx = m_mouseDeltaX;
    dy = m_mouseDeltaY;
}

void Input::ResetMouseDelta() {
    m_mouseDeltaX = 0.0f;
    m_mouseDeltaY = 0.0f;
}

void Input::SetMouseScrollDelta(float delta) {
    m_mouseScrollDelta += delta;
}

float Input::GetMouseScrollDelta() const {
    return m_mouseScrollDelta;
}

void Input::ResetMouseScroll() {
    m_mouseScrollDelta = 0.0f;
}

void Input::Update() {
    // Save current states as previous
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;

    // Reset scroll delta
    ResetMouseScroll();
}

// Key name helpers for debugging
std::string Input::GetKeyName(SDL_Scancode key) {
    const char* name = SDL_GetScancodeName(key);
    return name ? std::string(name) : "Unknown";
}

// Check for common key combinations
bool Input::IsMovementKey(SDL_Scancode key) {
    return key == SDL_SCANCODE_W || key == SDL_SCANCODE_A ||
           key == SDL_SCANCODE_S || key == SDL_SCANCODE_D ||
           key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_LSHIFT;
}

bool Input::IsAnyMovementKeyDown() const {
    return IsKeyDown(SDL_SCANCODE_W) || IsKeyDown(SDL_SCANCODE_A) ||
           IsKeyDown(SDL_SCANCODE_S) || IsKeyDown(SDL_SCANCODE_D) ||
           IsKeyDown(SDL_SCANCODE_SPACE) || IsKeyDown(SDL_SCANCODE_LSHIFT);
}

// Get movement vector from WASD keys
glm::vec2 Input::GetMovementVector() const {
    glm::vec2 movement(0.0f);

    if (IsKeyDown(SDL_SCANCODE_W)) movement.y += 1.0f;
    if (IsKeyDown(SDL_SCANCODE_S)) movement.y -= 1.0f;
    if (IsKeyDown(SDL_SCANCODE_D)) movement.x += 1.0f;
    if (IsKeyDown(SDL_SCANCODE_A)) movement.x -= 1.0f;

    // Normalize diagonal movement
    if (movement.x != 0.0f && movement.y != 0.0f) {
        movement = glm::normalize(movement);
    }

    return movement;
}

// Check for modifier keys
bool Input::IsShiftDown() const {
    return IsKeyDown(SDL_SCANCODE_LSHIFT) || IsKeyDown(SDL_SCANCODE_RSHIFT);
}

bool Input::IsCtrlDown() const {
    return IsKeyDown(SDL_SCANCODE_LCTRL) || IsKeyDown(SDL_SCANCODE_RCTRL);
}

bool Input::IsAltDown() const {
    return IsKeyDown(SDL_SCANCODE_LALT) || IsKeyDown(SDL_SCANCODE_RALT);
}