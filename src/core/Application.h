//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <memory>
#include <string>

#include "Camera.h"
#include "Input.h"
#include "../rendering/VoxelRenderer.h"
#include "../world/ChunkManager.h"

class Application {
public:
    Application();
    ~Application();

    bool Initialize(const std::string& title, uint32_t width, uint32_t height);
    void Run();
    void Shutdown();

private:
    bool InitializeSDL(const std::string& title);
    bool InitializeOpenGL();
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();
    void OnResize(uint32_t width, uint32_t height);

    // SDL & OpenGL
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;
    bool m_running = false;
    bool m_mouseCaptured = false;

    // Components
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<VoxelRenderer> m_renderer;
    std::unique_ptr<ChunkManager> m_chunkManager;

    // Timing
    uint64_t m_lastFrameTime;
    float m_deltaTime = 0.0f;
    float m_frameTime = 0.0f;

    // Performance counters
    uint32_t m_frameCount = 0;
    float m_fpsTimer = 0.0f;
    float m_currentFPS = 0.0f;
};