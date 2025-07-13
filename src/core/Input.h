//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class Input {
public:
    Input();
    ~Input() = default;

    // Keyboard input
    void SetKeyDown(SDL_Scancode key);
    void SetKeyUp(SDL_Scancode key);
    bool IsKeyDown(SDL_Scancode key) const;
    bool IsKeyPressed(SDL_Scancode key) const;  // Just pressed this frame
    bool IsKeyReleased(SDL_Scancode key) const; // Just released this frame

    // Mouse input
    void SetMouseButtonDown(uint8_t button);
    void SetMouseButtonUp(uint8_t button);
    bool IsMouseButtonDown(uint8_t button) const;
    bool IsMouseButtonPressed(uint8_t button) const;
    bool IsMouseButtonReleased(uint8_t button) const;

    // Mouse position and movement
    void SetMousePosition(int x, int y);
    void SetMouseDelta(int dx, int dy);
    void GetMousePosition(int& x, int& y) const;
    void GetMouseDelta(float& dx, float& dy) const;
    void ResetMouseDelta();

    // Mouse scroll
    void SetMouseScrollDelta(float delta);
    float GetMouseScrollDelta() const;
    void ResetMouseScroll();

    // Update state (call each frame)
    void Update();

    // Helper functions
    std::string GetKeyName(SDL_Scancode key);
    bool IsMovementKey(SDL_Scancode key);
    bool IsAnyMovementKeyDown() const;
    glm::vec2 GetMovementVector() const;

    // Modifier keys
    bool IsShiftDown() const;
    bool IsCtrlDown() const;
    bool IsAltDown() const;

    // Common key constants
    static constexpr SDL_Scancode KEY_FORWARD = SDL_SCANCODE_W;
    static constexpr SDL_Scancode KEY_BACKWARD = SDL_SCANCODE_S;
    static constexpr SDL_Scancode KEY_LEFT = SDL_SCANCODE_A;
    static constexpr SDL_Scancode KEY_RIGHT = SDL_SCANCODE_D;
    static constexpr SDL_Scancode KEY_UP = SDL_SCANCODE_SPACE;
    static constexpr SDL_Scancode KEY_DOWN = SDL_SCANCODE_LSHIFT;
    static constexpr SDL_Scancode KEY_SPRINT = SDL_SCANCODE_LCTRL;
    static constexpr SDL_Scancode KEY_JUMP = SDL_SCANCODE_SPACE;

    // Mouse button constants
    static constexpr uint8_t MOUSE_LEFT = SDL_BUTTON_LEFT;
    static constexpr uint8_t MOUSE_RIGHT = SDL_BUTTON_RIGHT;
    static constexpr uint8_t MOUSE_MIDDLE = SDL_BUTTON_MIDDLE;

private:
    // Key states
    std::unordered_map<SDL_Scancode, bool> m_currentKeys;
    std::unordered_map<SDL_Scancode, bool> m_previousKeys;

    // Mouse button states
    std::unordered_map<uint8_t, bool> m_currentMouseButtons;
    std::unordered_map<uint8_t, bool> m_previousMouseButtons;

    // Mouse position and movement
    int m_mouseX = 0;
    int m_mouseY = 0;
    float m_mouseDeltaX = 0.0f;
    float m_mouseDeltaY = 0.0f;
    float m_mouseScrollDelta = 0.0f;
};