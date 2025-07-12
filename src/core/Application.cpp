//
// Created by mrsomfergo on 12.07.2025.
//

#include "Application.h"
#include <iostream>
#include <vector>
#include <set>

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

    if (!InitializeVulkan()) {
        return false;
    }

    if (!CreateNVRHIDevice()) {
        return false;
    }

    // Создаем компоненты
    m_camera = std::make_unique<Camera>(60.0f, (float)width / height, 0.1f, 1000.0f);
    m_input = std::make_unique<Input>();
    m_renderer = std::make_unique<VoxelRenderer>(m_nvrhiDevice.Get(), m_commandList.Get());
    m_chunkManager = std::make_unique<ChunkManager>();

    // Инициализируем рендерер
    m_renderer->Initialize(width, height, m_chunkManager.get());

    m_lastFrameTime = SDL_GetPerformanceCounter();
    m_running = true;

    return true;
}

bool Application::InitializeSDL(const std::string& title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(
        title.c_str(),
        m_windowWidth, m_windowHeight,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Захват мыши для FPS камеры
    // SDL_SetRelativeMouseMode(SDL_TRUE);

    return true;
}

bool Application::InitializeVulkan() {
    // Получаем расширения, требуемые SDL
    unsigned int extensionCount = 0;
    char const* const* extensionNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    if (!extensionNames) {
        // Обработка ошибки
        throw std::runtime_error(SDL_GetError());
    }

    // Если вам нужен std::vector, создайте его из полученного массива
    std::vector<const char*> extensions(extensionNames, extensionNames + extensionCount);

    // Создаем Vulkan instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Voxel Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Validation layers в debug режиме
#ifdef _DEBUG
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return false;
    }

    // Создаем surface
    if (!SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface)) {
        std::cerr << "Failed to create Vulkan surface!" << std::endl;
        return false;
    }

    // Выбираем физическое устройство
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Выбираем первое дискретное GPU или первое доступное
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || m_physicalDevice == VK_NULL_HANDLE) {
            m_physicalDevice = device;
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                break;
            }
        }
    }

    // Находим графическую очередь
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
            m_graphicsQueueFamily = i;
            break;
        }
    }

    // Создаем логическое устройство
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
        std::cerr << "Failed to create logical device!" << std::endl;
        return false;
    }

    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);

    return true;
}

bool Application::CreateNVRHIDevice() {
    // Создаем NVRHI device manager
    nvrhi::vulkan::DeviceDesc deviceDesc;
    deviceDesc.instance = m_instance;
    deviceDesc.physicalDevice = m_physicalDevice;
    deviceDesc.device = m_device;
    deviceDesc.graphicsQueue = m_graphicsQueue;
    deviceDesc.graphicsQueueIndex = m_graphicsQueueFamily;

    m_deviceHandle = createDevice(deviceDesc);
    m_commandList = m_deviceHandle->createCommandList();

    // Создаем swapchain
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = 3;
    swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = { m_windowWidth, m_windowHeight };
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainInfo.clipped = VK_TRUE;

    vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);

    // Получаем изображения swapchain
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    // Создаем NVRHI текстуры для swapchain
    for (VkImage image : m_swapchainImages) {
        // nvrhi::TextureDesc textureDesc;
        // textureDesc.format = nvrhi::Format::BGRA8_UNORM;
        // textureDesc.width = m_windowWidth;
        // textureDesc.height = m_windowHeight;
        // textureDesc.isRenderTarget = true;
        auto textureDesc = nvrhi::TextureDesc()
            .setDimension(nvrhi::TextureDimension::Texture2D)
            .setFormat(nvrhi::Format::RGBA8_UNORM)
            .setWidth(m_windowWidth)
            .setHeight(m_windowHeight)
            .setIsRenderTarget(true)
            .setDebugName("Swap Chain Image");
        auto texture = m_nvrhiDevice->createHandleForNativeTexture(
            nvrhi::ObjectTypes::VK_Image, nvrhi::Object(image), textureDesc);
        m_swapchainTextures.push_back(texture);
    }

    // Создаем depth буфер
    nvrhi::TextureDesc depthDesc;
    depthDesc.format = nvrhi::Format::D32;
    depthDesc.width = m_windowWidth;
    depthDesc.height = m_windowHeight;
    depthDesc.isRenderTarget = true;
    depthDesc.initialState = nvrhi::ResourceStates::DepthWrite;
    depthDesc.keepInitialState = true;
    m_depthTexture = m_nvrhiDevice->createTexture(depthDesc);

    // Создаем framebuffers
    for (auto& texture : m_swapchainTextures) {
        nvrhi::FramebufferDesc fbDesc;
        fbDesc.addColorAttachment(texture);
        fbDesc.setDepthAttachment(m_depthTexture);
        m_swapchainFramebuffers.push_back(m_nvrhiDevice->createFramebuffer(fbDesc));
    }

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
                m_input->SetKeyDown(event.key.scancode);
                break;

            case SDL_EVENT_KEY_UP:
                m_input->SetKeyUp(event.key.scancode);
                break;

            case SDL_EVENT_MOUSE_MOTION:
                m_input->SetMouseDelta(event.motion.xrel, event.motion.yrel);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                m_input->SetMouseButtonDown(event.button.button);
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                m_input->SetMouseButtonUp(event.button.button);
                break;
        }
    }
}

void Application::Update(float deltaTime) {
    // Обновляем камеру на основе ввода
    const float moveSpeed = 10.0f;
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

    // Вращение камеры мышью
    float mouseX, mouseY;
    m_input->GetMouseDelta(mouseX, mouseY);
    m_camera->Rotate(-mouseX * mouseSensitivity, -mouseY * mouseSensitivity);

    // Обновляем менеджер чанков
    m_chunkManager->Update(m_camera->GetPosition(), deltaTime);

    // Обновляем рендерер
    m_renderer->UpdateCamera(m_camera.get());

    // Сбрасываем дельту мыши
    m_input->ResetMouseDelta();
}

void Application::Render() {
    // Получаем следующее изображение из swapchain
    VkSemaphore imageAvailable;
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &imageAvailable);

    vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &m_currentSwapchainIndex);

    // Очищаем экран
    m_commandList->clearTextureFloat(m_swapchainTextures[m_currentSwapchainIndex],
                                    nvrhi::AllSubresources, nvrhi::Color(0.529f, 0.808f, 0.922f, 1.0f));
    m_commandList->clearDepthStencilTexture(m_depthTexture, nvrhi::AllSubresources, true, 1.0f, false, 0);

    // Рендерим
    m_renderer->Render(m_swapchainFramebuffers[m_currentSwapchainIndex]);

    // Выполняем команды
    m_nvrhiDevice->executeCommandList(m_commandList);

    // Представляем кадр
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &imageAvailable;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentSwapchainIndex;

    vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
    vkQueueWaitIdle(m_graphicsQueue);

    vkDestroySemaphore(m_device, imageAvailable, nullptr);
}

void Application::Run() {
    while (m_running) {
        // Вычисляем delta time
        uint64_t currentTime = SDL_GetPerformanceCounter();
        m_deltaTime = (float)(currentTime - m_lastFrameTime) / SDL_GetPerformanceFrequency();
        m_lastFrameTime = currentTime;

        ProcessEvents();
        Update(m_deltaTime);
        Render();
    }
}

void Application::Shutdown() {
    // Ждем завершения всех операций
    vkDeviceWaitIdle(m_device);

    // Очищаем NVRHI ресурсы
    m_swapchainFramebuffers.clear();
    m_swapchainTextures.clear();
    m_depthTexture = nullptr;
    m_commandList = nullptr;
    m_nvrhiDevice = nullptr;

    // Очищаем компоненты
    m_renderer.reset();
    m_chunkManager.reset();
    m_input.reset();
    m_camera.reset();

    // Очищаем Vulkan
    if (m_swapchain) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }

    delete m_deviceHandle;

    if (m_surface) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }
    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
    }
    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
    }

    // Очищаем SDL
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}
