//
// Created by mrsomfergo on 12.07.2025.
//

#include "Input.h"

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
    m_mouseDeltaX += (float)dx;
    m_mouseDeltaY += (float)dy;
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

void Input::Update() {
    // Сохраняем текущие состояния как предыдущие
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;
}