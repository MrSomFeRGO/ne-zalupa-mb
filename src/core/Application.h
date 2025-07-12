//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>
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
    bool InitializeVulkan();
    bool CreateNVRHIDevice();
    void ProcessEvents();
    void Update(float deltaTime);
    void Render();

    // SDL
    SDL_Window* m_window = nullptr;
    uint32_t m_windowWidth;
    uint32_t m_windowHeight;
    bool m_running = false;

    // Vulkan
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;

    // NVRHI
    nvrhi::vulkan::DeviceHandle m_nvrhiDevice;
    nvrhi::CommandListHandle m_commandList;
    nvrhi::DeviceHandle m_deviceHandle = nullptr;

    // Swapchain
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<nvrhi::TextureHandle> m_swapchainTextures;
    std::vector<nvrhi::FramebufferHandle> m_swapchainFramebuffers;
    nvrhi::TextureHandle m_depthTexture;
    uint32_t m_currentSwapchainIndex = 0;

    // Components
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<VoxelRenderer> m_renderer;
    std::unique_ptr<ChunkManager> m_chunkManager;

    // Timing
    uint64_t m_lastFrameTime;
    float m_deltaTime = 0.0f;
};
