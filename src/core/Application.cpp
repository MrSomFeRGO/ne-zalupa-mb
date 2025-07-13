//
// Created by mrsomfergo on 13.07.2025.
//

#include "Application.h"
#include "../rendering/OpenGLUtils.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Application::Application() {
}

Application::~Application() {
}

bool Application::Initialize(const std::string& title, uint32_t width, uint32_t height) {
    m_windowWidth = width;
    m_windowHeight = height;

    if (!InitializeSDL(title)) {
        return false;
    }

    if (!InitializeOpenGL()) {
        return false;
    }

    // Create components
    m_camera = std::make_unique<Camera>(60.0f, (float)width / height, 0.1f, 1000.0f);
    m_input = std::make_unique<Input>();
    m_renderer = std::make_unique<VoxelRenderer>();
    m_chunkManager = std::make_unique<ChunkManager>();

    // Initialize components
    m_chunkManager->Initialize();
    m_renderer->Initialize(width, height, m_chunkManager.get());
    m_renderer->UpdateCamera(m_camera.get());

    // Setup initial camera position
    m_camera->SetPosition(glm::vec3(0.0f, 20.0f, 0.0f));

    m_lastFrameTime = SDL_GetPerformanceCounter();
    m_running = true;

    std::cout << "VoxelEngine initialized successfully!" << std::endl;
    std::cout << "Note: OpenGL objects are created in main thread (no more SIGSEGV!)" << std::endl;

    return true;
}

bool Application::InitializeSDL(const std::string& title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_window = SDL_CreateWindow(
        title.c_str(),
        m_windowWidth, m_windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Application::InitializeOpenGL() {
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Make context current
    if (!SDL_GL_MakeCurrent(m_window, m_glContext)) {
        std::cerr << "SDL_GL_MakeCurrent Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Load OpenGL functions with GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Check OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;

    // Check if we have OpenGL 4.5
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    if (majorVersion < 4 || (majorVersion == 4 && minorVersion < 5)) {
        std::cerr << "OpenGL 4.5 or higher required, but got " << majorVersion << "." << minorVersion << std::endl;
        return false;
    }

    // Enable debug output if available
    #ifdef _DEBUG
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::cout << "OpenGL debug output enabled" << std::endl;
    }
    #endif

    // Enable VSync
    if (SDL_GL_SetSwapInterval(1) != 0) {
        std::cout << "Warning: VSync not supported" << std::endl;
    }

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set viewport
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    // Check for errors
    CheckGLError("OpenGL initialization");

    return true;
}

void Application::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    m_running = false;
                }
                if (event.key.key == SDLK_TAB) {
                    m_mouseCaptured = !m_mouseCaptured;
                    SDL_SetWindowRelativeMouseMode(m_window, m_mouseCaptured);
                }
                m_input->SetKeyDown(event.key.scancode);
                break;

            case SDL_EVENT_KEY_UP:
                m_input->SetKeyUp(event.key.scancode);
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (m_mouseCaptured) {
                    m_input->SetMouseDelta(event.motion.xrel, event.motion.yrel);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                m_input->SetMouseButtonDown(event.button.button);
                if (!m_mouseCaptured) {
                    m_mouseCaptured = true;
                    SDL_SetWindowRelativeMouseMode(m_window, true);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                m_input->SetMouseButtonUp(event.button.button);
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                OnResize(event.window.data1, event.window.data2);
                break;
        }
    }
}

void Application::Update(float deltaTime) {
    // Camera movement
    const float moveSpeed = 20.0f;
    const float mouseSensitivity = 0.002f;

    if (m_input->IsKeyDown(SDL_SCANCODE_W)) {
        m_camera->MoveForward(moveSpeed * deltaTime);
    }
    if (m_input->IsKeyDown(SDL_SCANCODE_S)) {
        m_camera->MoveForward(-moveSpeed * deltaTime);
    }
    if (m_input->IsKeyDown(SDL_SCANCODE_A)) {
        m_camera->MoveRight(-moveSpeed * deltaTime);
    }
    if (m_input->IsKeyDown(SDL_SCANCODE_D)) {
        m_camera->MoveRight(moveSpeed * deltaTime);
    }
    if (m_input->IsKeyDown(SDL_SCANCODE_SPACE)) {
        m_camera->MoveUp(moveSpeed * deltaTime);
    }
    if (m_input->IsKeyDown(SDL_SCANCODE_LSHIFT)) {
        m_camera->MoveUp(-moveSpeed * deltaTime);
    }

    // Camera rotation
    if (m_mouseCaptured) {
        float mouseX, mouseY;
        m_input->GetMouseDelta(mouseX, mouseY);
        m_camera->Rotate(-mouseX * mouseSensitivity, -mouseY * mouseSensitivity);
    }

    // Update world
    m_chunkManager->Update(m_camera->GetPosition(), deltaTime);

    // Update renderer
    m_renderer->UpdateCamera(m_camera.get());

    // Reset input
    m_input->ResetMouseDelta();
    m_input->Update();

    // Update FPS counter
    m_frameCount++;
    m_fpsTimer += deltaTime;
    if (m_fpsTimer >= 1.0f) {
        m_currentFPS = m_frameCount / m_fpsTimer;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;

        // Update window title with FPS
        std::ostringstream title;
        title << "VoxelEngine - FPS: " << std::fixed << std::setprecision(1) << m_currentFPS
              << " | Chunks: " << m_chunkManager->GetLoadedChunkCount()
              << " | Pos: (" << std::setprecision(1)
              << m_camera->GetPosition().x << ", "
              << m_camera->GetPosition().y << ", "
              << m_camera->GetPosition().z << ")";
        SDL_SetWindowTitle(m_window, title.str().c_str());
    }
}

void Application::Render() {
    // Clear screen
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render world
    m_renderer->Render();

    // Swap buffers
    SDL_GL_SwapWindow(m_window);

    CheckGLError("Render");
}

void Application::OnResize(uint32_t width, uint32_t height) {
    m_windowWidth = width;
    m_windowHeight = height;

    glViewport(0, 0, width, height);
    m_camera->SetAspectRatio((float)width / height);
    m_renderer->OnResize(width, height);
}

void Application::Run() {
    std::cout << "Starting game loop..." << std::endl;
    std::cout << "Press TAB to toggle mouse capture" << std::endl;
    std::cout << "Press ESC to exit" << std::endl;

    while (m_running) {
        // Calculate delta time
        uint64_t currentTime = SDL_GetPerformanceCounter();
        m_deltaTime = (float)(currentTime - m_lastFrameTime) / SDL_GetPerformanceFrequency();
        m_lastFrameTime = currentTime;

        // Cap delta time to prevent large jumps
        m_deltaTime = std::min(m_deltaTime, 0.1f);

        ProcessEvents();
        Update(m_deltaTime);
        Render();
    }
}

void Application::Shutdown() {
    std::cout << "Shutting down..." << std::endl;

    // Cleanup components
    m_renderer.reset();
    m_chunkManager.reset();
    m_input.reset();
    m_camera.reset();

    // Cleanup OpenGL & SDL
    if (m_glContext) {
        SDL_GL_DestroyContext(m_glContext);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();

    std::cout << "Goodbye!" << std::endl;
}