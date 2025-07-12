//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

class Input {
public:
    Input();
    ~Input() = default;

    // Клавиатура
    void SetKeyDown(SDL_Scancode key);
    void SetKeyUp(SDL_Scancode key);
    bool IsKeyDown(SDL_Scancode key) const;
    bool IsKeyPressed(SDL_Scancode key) const;  // Только что нажата
    bool IsKeyReleased(SDL_Scancode key) const; // Только что отпущена

    // Мышь
    void SetMouseButtonDown(uint8_t button);
    void SetMouseButtonUp(uint8_t button);
    bool IsMouseButtonDown(uint8_t button) const;
    bool IsMouseButtonPressed(uint8_t button) const;
    bool IsMouseButtonReleased(uint8_t button) const;

    void SetMousePosition(int x, int y);
    void SetMouseDelta(int dx, int dy);
    void GetMousePosition(int& x, int& y) const;
    void GetMouseDelta(float& dx, float& dy) const;
    void ResetMouseDelta();

    // Обновление состояний (вызывается каждый кадр)
    void Update();

private:
    // Состояния клавиш
    std::unordered_map<SDL_Scancode, bool> m_currentKeys;
    std::unordered_map<SDL_Scancode, bool> m_previousKeys;

    // Состояния кнопок мыши
    std::unordered_map<uint8_t, bool> m_currentMouseButtons;
    std::unordered_map<uint8_t, bool> m_previousMouseButtons;

    // Позиция и движение мыши
    int m_mouseX = 0;
    int m_mouseY = 0;
    float m_mouseDeltaX = 0.0f;
    float m_mouseDeltaY = 0.0f;
};