#include "vulkan_handle.hpp"
#include "types.hpp"
#include "result.hpp"

#include "vulkan/vulkan_core.h"
#include "SDL3/SDL_vulkan.h"
#include "vulkan/vulkan_wayland.h"
#include <glm/ext/vector_float2.hpp>

#if defined(OS_WINDOWS)
#define NOMINMAX
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#endif
#include <SDL3/SDL_platform.h>
#include <SDL3/SDL_platform_defines.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <limits>
#include <algorithm>
#include <fstream>

#undef ERROR

using namespace lle;

#if (DEBUG == true)
const b8_t VulkanHandle::sEnableValidation_ = true;
#else
const b8_t VulkanHandle::sEnableValidation_ = false;
#endif

bool contains(const std::vector<const char*>& vec, const char* str) {
    return std::any_of(vec.begin(), vec.end(),
        [&](const char* s) { return std::strcmp(s, str) == 0; });
}

b8_t QueueCollection::isComplete() {
    return graphicsIndex.has_value() && presentIndex.has_value();
}

i32_t VulkanHandle::sPicDefaultGPU_(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features) {
    i8_t score = 0;

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 50;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 25;
        break;
    default:
        break;
    }

    return score;
}



constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
size_t frameIndex = 0;



const std::vector<cstr_t> VulkanHandle::sRequiredValidationLayers_{
    
    "VK_LAYER_KHRONOS_validation",
};

const std::unordered_map<std::string, std::vector<cstr_t>> VulkanHandle::sRequiredVkInstExtensions_{
    {"Linux", {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME}}, 
    {"Windows", {VK_KHR_SURFACE_EXTENSION_NAME}}
};

const std::vector<cstr_t> VulkanHandle::sRequiredVkDevExtensions_{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};



Result VulkanHandle::AutoInit() noexcept {

    if (Result res = mCreateInst_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan instance created" << std::endl;)

    if (Result res = mCreateSurface_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan surface created" << std::endl;)

    if (Result res = mListGPUs_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan GPUs queried" << std::endl;)

    if (Result res = mChooseGPU_(VulkanHandle::sPicDefaultGPU_); res != Result::LFS_SUCCESS) return res;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(*mPhysicalDevice_, &properties);
    NO_ERR_DBG_BLOCK(std::cout << "--> GPU selected: " << properties.deviceName << '\n';)

    if (Result res = mSetupVulkan_(); res != Result::LFS_SUCCESS) return res;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateInst_() noexcept {

    u32_t instExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instExtensionCount, nullptr);

    std::vector<VkExtensionProperties> instExtProperties(instExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instExtensionCount, instExtProperties.data());

    cstr_t platform = SDL_GetPlatform();

    if (!VulkanHandle::sRequiredVkInstExtensions_.contains(std::string(platform))) return PLATFORM_NO_SUPPORT;
    
    for (const cstr_t &extension : sRequiredVkInstExtensions_.at(platform)) {
        b8_t supported = false;

        for (const VkExtensionProperties &extensionProperty : instExtProperties) {
            if (strcmp(extension, extensionProperty.extensionName) == 0) {
                supported = true;
                mInstanceExtensions_.push_back(extension);
                break;
            }
        }
        if (!supported) return INST_EXT_NO_SUPPORT;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = mAppName_;
    appInfo.applicationVersion = mVersion_.ToVulkan();
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    u32_t extensionCount = 0;
    const cstr_t *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    for (size_t i = 0; i < extensionCount; ++i) {
        if (!contains(mInstanceExtensions_, extensions[i])) {
            mInstanceExtensions_.push_back(extensions[i]);
        }
    }

    createInfo.enabledExtensionCount = static_cast<u32_t>(mInstanceExtensions_.size());
    createInfo.ppEnabledExtensionNames = mInstanceExtensions_.data();

    u32_t enabledLayerCount = 0;
    if (sEnableValidation_) {
        if (Result res = mCheckValidationLayers_(); res != Result::LFS_SUCCESS) return res;
        else {
            enabledLayerCount = static_cast<u32_t>(sRequiredValidationLayers_.size());
            createInfo.ppEnabledLayerNames = sRequiredValidationLayers_.data();
        }
    }
    createInfo.enabledLayerCount = enabledLayerCount;

    if (vkCreateInstance(&createInfo, nullptr, &mVkInst_) != VK_SUCCESS) return INST_CREATE_ERR;

    uint32_t version = 0;
    vkEnumerateInstanceVersion(&version);

    return Result::LFS_SUCCESS;
}


Result VulkanHandle::mCheckValidationLayers_() noexcept {
    u32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const cstr_t layerName : sRequiredValidationLayers_) {
        b8_t layerFound = false;

        for (VkLayerProperties &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
            break;
            }
        }

        if (!layerFound) return VAL_LAYER_NO_SUPPORT;
    }

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mSetupVulkan_() noexcept {
    if (mPhysicalDevice_ == nullptr) return GPU_NOT_DEFINED;

    if (Result res = mCreateLogicalDevice_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan logical device created" << std::endl;)

    if (Result res = mCreateQueues_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan graphics and present queues created" << std::endl;)

    if (Result res = mCreateSwapchain_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan swap chain created [" << mSwapchainExtent_.width << ", " << mSwapchainExtent_.height << "]" << std::endl;)

    if (Result res = mCreateRenderPass_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan render pass created" << std::endl;)

    if (Result res = mCreateFrameBuffers_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan frame buffers created" << std::endl;)

    if (Result res = mCreateCommandPool_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan command pool created" << std::endl;)

    if (Result res = mCreateDescriptorPool_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan descriptor pool created" << std::endl;)

    if (Result res = mCreateDescriptorLayout_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan descriptor set layout created" << std::endl;)

    if (Result res = mCreatePipelineLayout_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan graphics pipeline layout created" << std::endl;)

    if (Result res = mCreateSyncObjects_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan semapores and fence created" << std::endl;)


    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan instance extensions:" << std::endl;
    for (auto ext : mInstanceExtensions_) {
        std::cout << "    - " << ext << '\n';
    })

    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan device extensions:" << std::endl;
    for (auto ext : mDeviceExtensions_) {
        std::cout << "    - " << ext << '\n';
    })

    NO_ERR_DBG_BLOCK(std::cout << '\n';)

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateSurface_() noexcept {

    if (!SDL_Vulkan_CreateSurface(*mSDL3window_, mVkInst_, nullptr, &mVkSurface_)) return SURFACE_CREATE_ERR;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mListGPUs_() noexcept {

    u32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mVkInst_, &deviceCount, nullptr);
    if (deviceCount == 0) return NO_GPUS_FOUND;
    mPhysicalDevices_.resize(deviceCount);
    vkEnumeratePhysicalDevices(mVkInst_, &deviceCount, mPhysicalDevices_.data());

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mChooseGPU_(i32_t (*function)(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features)) noexcept {

    if (mVkInst_ == nullptr) return INST_NOT_DEFINED;
    if (mPhysicalDevices_.size() == 0) return NO_GPUS_FOUND;

    std::unordered_map<i8_t, VkPhysicalDevice*> scores;

    for (size_t i = 0; i < mPhysicalDevices_.size(); ++i) {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(mPhysicalDevices_[i], &properties);
        vkGetPhysicalDeviceFeatures(mPhysicalDevices_[i], &features);

        scores[function(properties, features)] = &mPhysicalDevices_[i];

    }

    std::unordered_map<i8_t, VkPhysicalDevice*>::iterator iterator = scores.begin();
    i8_t highestScore = iterator->first;
    iterator++;

    for (; iterator != scores.end(); ++iterator) {
        if (iterator->first > highestScore) highestScore = iterator->first;
    }

    if (highestScore < 0) return NO_GPU_SUITABLE;
    else mPhysicalDevice_ = scores[highestScore];

    if (Result res = mCheckQueueFamilySupport_(); res != Result::LFS_SUCCESS) return res;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCheckQueueFamilySupport_() noexcept {
    if (mPhysicalDevice_ == nullptr) return GPU_NOT_DEFINED;

    u32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*mPhysicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(*mPhysicalDevice_, &queueFamilyCount, queueFamilies.data());

    u32_t i = 0;

    for (VkQueueFamilyProperties &queueProperty : queueFamilies) {
        if (queueProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) { 
            if (auto element = mQueues_.indeces.find(i); element != mQueues_.indeces.end()) {
                mQueues_.graphicsIndex = &(*element);
            } else {
                auto [it, _] = mQueues_.indeces.insert(i);
                mQueues_.graphicsIndex = &(*it);
            }
        }

        VkBool32 presentSupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(*mPhysicalDevice_, i, mVkSurface_, &presentSupport) != VK_SUCCESS) continue;
        if (presentSupport) {
            if (std::set<u32_t>::iterator element = mQueues_.indeces.find(i); element != mQueues_.indeces.end()) {
                mQueues_.presentIndex = &*element;
            } else {
                auto [it, inserted] = mQueues_.indeces.insert(i);
                mQueues_.presentIndex = &*it;
            }
        }

        if (mQueues_.isComplete()) return Result::LFS_SUCCESS;
        i++;
    }

    return QUEUE_FAMILIES_NOT_SUFFICIENT;
}



Result VulkanHandle::mCreateLogicalDevice_() noexcept {
    if (mPhysicalDevice_ == nullptr) return GPU_NOT_DEFINED;

    vkGetPhysicalDeviceMemoryProperties(*mPhysicalDevice_, &mDeviceMemProperties_);

    if (!mQueues_.isComplete()) return QUEUE_FAMILIES_NOT_SUFFICIENT;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(mQueues_.indeces.size());

    u32_t i = 0;
    f32_t queuePriority = 1.0;
    for (auto queueIndex : mQueues_.indeces) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = static_cast<u32_t>(queueIndex);
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[0] = queueCreateInfo;
    }
    
    u32_t devExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(*mPhysicalDevice_, nullptr, &devExtensionCount, nullptr);

    std::vector<VkExtensionProperties> devExtProperties(devExtensionCount);
    vkEnumerateDeviceExtensionProperties(*mPhysicalDevice_, nullptr, &devExtensionCount, devExtProperties.data());

    for (const cstr_t &extension : sRequiredVkDevExtensions_) {
        b8_t supported = false;

        for (const VkExtensionProperties &extensionProperty : devExtProperties) {
            if (strcmp(extension, extensionProperty.extensionName) == 0) {
                supported = true;
                mDeviceExtensions_.push_back(extension);
                break;
            }
        }
        if (!supported) return DEV_EXT_NO_SUPPORT;
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<u32_t>(mQueues_.indeces.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<u32_t>(mDeviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = mDeviceExtensions_.data();

    /*if (sEnableValidation_) {
        createInfo.enabledLayerCount = static_cast<u32_t>(sRequiredValidationLayers_.size());
        createInfo.ppEnabledLayerNames = sRequiredValidationLayers_.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }*/
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    if (vkCreateDevice(*mPhysicalDevice_, &createInfo, nullptr, &mLogicalDevice_) != VK_SUCCESS) return DEV_CREATE_ERR;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateQueues_() noexcept {

    vkGetDeviceQueue(mLogicalDevice_, *mQueues_.graphicsIndex.value(), 0, &mQueues_.graphicsQueue);
    vkGetDeviceQueue(mLogicalDevice_, *mQueues_.presentIndex.value(), 0, &mQueues_.presentQueue);

    return Result::LFS_SUCCESS;
}


VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window *mSDL3window_) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSizeInPixels(mSDL3window_, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<u32_t>(width),
            static_cast<u32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

Result VulkanHandle::mCreateSwapchain_() noexcept {

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;


    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*mPhysicalDevice_, mVkSurface_, &capabilities);

    u32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*mPhysicalDevice_, mVkSurface_, &formatCount, nullptr);

    if (formatCount == 0) return NO_SWAPCHAIN_FORMATS;
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*mPhysicalDevice_, mVkSurface_, &formatCount, formats.data());


    u32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*mPhysicalDevice_, mVkSurface_, &presentModeCount, nullptr);

    if (presentModeCount == 0) return NO_SWAPCHAIN_PRESENT_MODES;
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(*mPhysicalDevice_, mVkSurface_, &presentModeCount, presentModes.data());

    VkExtent2D extent = chooseSwapExtent(capabilities, *mSDL3window_);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);

    u32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mVkSurface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32_t queuIndeces[] = {*mQueues_.graphicsIndex.value(), *mQueues_.presentIndex.value()};

    if (*mQueues_.graphicsIndex.value() != *mQueues_.presentIndex.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queuIndeces;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mLogicalDevice_, &createInfo, nullptr, &mSwapchain_) != VK_SUCCESS) return SWAPCHAIN_CREATE_ERR;

    vkGetSwapchainImagesKHR(mLogicalDevice_, mSwapchain_, &imageCount, nullptr);
    mSwapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(mLogicalDevice_, mSwapchain_, &imageCount, mSwapchainImages_.data());

    mSwapchainImageFormat_ = surfaceFormat.format;
    mSwapchainExtent_ = extent;

    if (Result res = mCreateImageViews_(); res != Result::LFS_SUCCESS) return res;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateImageViews_() noexcept {

    mSwapchainImageViews_.resize(mSwapchainImages_.size());

    for (size_t i = 0; i < mSwapchainImages_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mSwapchainImages_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapchainImageFormat_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(mLogicalDevice_, &createInfo, nullptr, &mSwapchainImageViews_[i]) != VK_SUCCESS) return IMG_VIEW_CREATE_ERR;
    }

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateRenderPass_() noexcept {

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapchainImageFormat_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mLogicalDevice_, &renderPassCreateInfo, nullptr, &mRenderPass_) != VK_SUCCESS) return RENDER_PASS_CREATE_ERR;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateFrameBuffers_() noexcept {

    mFramebuffers_.resize(mSwapchainImageViews_.size());

    for (size_t i = 0; i < mSwapchainImageViews_.size(); i++) {

        VkImageView attachments[] = {
            mSwapchainImageViews_[i]
        };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = mRenderPass_;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = mSwapchainExtent_.width;
        framebufferCreateInfo.height = mSwapchainExtent_.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(mLogicalDevice_, &framebufferCreateInfo, nullptr, &mFramebuffers_[i]) != VK_SUCCESS) return FRAME_BUF_CREATE_ERR;
    }

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateCommandPool_() noexcept {

    VkCommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = *mQueues_.graphicsIndex.value();

    if (vkCreateCommandPool(mLogicalDevice_, &cmdPoolCreateInfo, nullptr, &mCommandPool_) != VK_SUCCESS) return CMD_POOL_CREATE_ERR;

    mCommandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateDescriptorPool_() noexcept {

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT * 10;

    VkDescriptorPoolCreateInfo descrPoolCreateInfo{};
    descrPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descrPoolCreateInfo.poolSizeCount = 1;
    descrPoolCreateInfo.pPoolSizes = &poolSize;
    descrPoolCreateInfo.maxSets = poolSize.descriptorCount;

    if (vkCreateDescriptorPool(mLogicalDevice_, &descrPoolCreateInfo, nullptr, &mDescriptorPool_) != VK_SUCCESS) return LFS_ERROR;

    mMemBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    mDescriptorSets_.resize(MAX_FRAMES_IN_FLIGHT);
    
    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateDescriptorLayout_() noexcept {

    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    if (vkCreateDescriptorSetLayout(mLogicalDevice_, &layoutCreateInfo, nullptr, &mDescriptorSetLayout_) != VK_SUCCESS) return LFS_ERROR;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreateSyncObjects_() noexcept {

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    mImageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores_.resize(mSwapchainImages_.size());
    mInFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < mSwapchainImages_.size(); i++) {
        if (vkCreateSemaphore(mLogicalDevice_, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphores_[i]) != VK_SUCCESS) return SEMAPHORE_CREATE_ERR;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(mLogicalDevice_, &semaphoreCreateInfo, nullptr, &mImageAvailableSemaphores_[i]) != VK_SUCCESS) return SEMAPHORE_CREATE_ERR;
    if (vkCreateFence(mLogicalDevice_, &fenceCreateInfo, nullptr, &mInFlightFences_[i]) != VK_SUCCESS) return FENCE_CREATE_ERR;
    }

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::mCreatePipelineLayout_() noexcept {

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(VkExtent2D);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &mDescriptorSetLayout_;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushRange;

    if (vkCreatePipelineLayout(mLogicalDevice_, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout_) != VK_SUCCESS) return PIPELINE_LAYOUT_CREATE_ERR;

    return Result::LFS_SUCCESS;
}



std::vector<char> readFile(cstr_t filename) {

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) return std::vector<char>{};

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

Result VulkanHandle::mCreateGraphicsPipeline_(std::string name, cstr_t vertexShaderPath, cstr_t fragmentShaderPath) noexcept {

    std::vector<char> vertexShaderCode = readFile(vertexShaderPath);
    std::vector<char> fragmentShaderCode = readFile(fragmentShaderPath);

    if (vertexShaderCode.empty() || fragmentShaderCode.empty()) return SHADER_NO_LOAD;


    VkShaderModuleCreateInfo vertexCreateInfo{};
    vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexCreateInfo.codeSize = vertexShaderCode.size();
    vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderCode.data());

    VkShaderModuleCreateInfo fragmentCreateInfo{};
    fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentCreateInfo.codeSize = fragmentShaderCode.size();
    fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderCode.data());

    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;

    if (vkCreateShaderModule(mLogicalDevice_, &vertexCreateInfo, nullptr, &vertexShader) != VK_SUCCESS) return SHADER_CREATE_ERR;
    if (vkCreateShaderModule(mLogicalDevice_, &fragmentCreateInfo, nullptr, &fragmentShader) != VK_SUCCESS) return SHADER_CREATE_ERR;

    VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
    vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageCreateInfo.module = vertexShader;
    vertexStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{};
    fragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageCreateInfo.module = fragmentShader;
    fragmentStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStageCreateInfo, fragmentStageCreateInfo};


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mSwapchainExtent_.width;
    viewport.height = (float)mSwapchainExtent_.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mSwapchainExtent_;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<u32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo{};
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

    pipelineCreateInfo.layout = mPipelineLayout_;
    pipelineCreateInfo.renderPass = mRenderPass_;
    pipelineCreateInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(mLogicalDevice_, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipelines_[name]) != VK_SUCCESS) return PIPELINE_CREATE_ERR;

    NO_ERR_DBG_BLOCK(std::cout << "--> Graphics Pipeline [" << name << "] created" << std::endl;)

    vkDestroyShaderModule(mLogicalDevice_, vertexShader, nullptr);
    vkDestroyShaderModule(mLogicalDevice_, fragmentShader, nullptr);
    return Result::LFS_SUCCESS;
}



Result getMemTypeIndex(VkPhysicalDeviceMemoryProperties devProperies, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags propertyFlags, u32_t *memTypeIndex) noexcept {

    for (size_t i = 0; i < devProperies.memoryTypeCount; i++) {

        if ((memRequirements.memoryTypeBits & 1 << i)
            && (devProperies.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
            *memTypeIndex = i;
            return Result::LFS_SUCCESS;
        }

    }

    return LFS_ERROR;
}



Result VulkanHandle::mDestroy_() noexcept {
    if (mVkInst_ == VK_NULL_HANDLE) return Result::LFS_SUCCESS;
    vkDeviceWaitIdle(mLogicalDevice_);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (auto &[name, buffer] : mMemBuffers_[i]) {
            vkDestroyBuffer(mLogicalDevice_, buffer.Buffer, nullptr);
            vkFreeMemory(mLogicalDevice_, buffer.Memory, nullptr);
        }
    }

    vkDestroyDescriptorSetLayout(mLogicalDevice_, mDescriptorSetLayout_, nullptr);
    vkDestroyDescriptorPool(mLogicalDevice_, mDescriptorPool_, nullptr);

    for (size_t i = 0; i < mSwapchainImages_.size(); i++) {
        vkDestroySemaphore(mLogicalDevice_, mRenderFinishedSemaphores_[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(mLogicalDevice_, mImageAvailableSemaphores_[i], nullptr);
        vkDestroyFence(mLogicalDevice_, mInFlightFences_[i], nullptr);
    }

    vkDestroyCommandPool(mLogicalDevice_, mCommandPool_, nullptr);

    for (auto framebuffer : mFramebuffers_) {
        vkDestroyFramebuffer(mLogicalDevice_, framebuffer, nullptr);
    }

    for (auto &[name, pipeline] : mGraphicsPipelines_) {
        vkDestroyPipeline(mLogicalDevice_, pipeline, nullptr);
    }

    vkDestroyPipelineLayout(mLogicalDevice_, mPipelineLayout_, nullptr);
    vkDestroyRenderPass(mLogicalDevice_, mRenderPass_, nullptr);

    for (VkImageView &imageView : mSwapchainImageViews_) {
        vkDestroyImageView(mLogicalDevice_, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mLogicalDevice_, mSwapchain_, nullptr);

    vkDestroyDevice(mLogicalDevice_, nullptr);

    vkDestroySurfaceKHR(mVkInst_, mVkSurface_, nullptr);

    vkDestroyInstance(mVkInst_, nullptr);
    mVkInst_ = VK_NULL_HANDLE;

    return Result::LFS_SUCCESS;
}



VulkanHandle::VulkanHandle() 
    : mVkInst_(nullptr),
      mVersion_(0, 0, 0) { }

VulkanHandle::VulkanHandle(const cstr_t appName, SDL_Window** window)
    : mVkInst_(),
      mAppName_(),
      mVersion_(0, 0, 0),
      mPhysicalDevices_(),
      mPhysicalDevice_(nullptr),
      mLogicalDevice_(),
      mSDL3window_(window) { }



VulkanHandle::~VulkanHandle() {

    mDestroy_();

}



Result VulkanHandle::ChooseGPU(const i32_t deviceIndex) noexcept {
    if (mVkInst_ == nullptr) return INST_NOT_DEFINED;
    if (mPhysicalDevices_.size() == 0) return GPU_NOT_DEFINED;

    mPhysicalDevice_ = &mPhysicalDevices_[deviceIndex];

    if (Result res = mCheckQueueFamilySupport_(); res != Result::LFS_SUCCESS) return res;

    if (Result res = mSetupVulkan_(); res != Result::LFS_SUCCESS) return res;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::ChooseGPU(i32_t (*function)(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features)) noexcept {

    if (Result res = mChooseGPU_(function); res != Result::LFS_SUCCESS) return res;

    if (Result res = mSetupVulkan_(); res != Result::LFS_SUCCESS) return res;

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::CreatePipeline(std::string name, cstr_t vertexShaderPath, cstr_t fragmentShaderPath) noexcept {
    return mCreateGraphicsPipeline_(name, vertexShaderPath, fragmentShaderPath);
}



Result VulkanHandle::CreateCommand(std::string name) noexcept {

    VkCommandBufferAllocateInfo cmdAllocInfo{};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = mCommandPool_;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkAllocateCommandBuffers(mLogicalDevice_, &cmdAllocInfo, &mCommandBuffers_[i][name]) != VK_SUCCESS) return CMD_BUF_ALLOC_ERR;
    }

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::DestroyCommand(std::string name) noexcept {

    return Result::LFS_SUCCESS;
}


Result VulkanHandle::RestartCommand(std::string name) noexcept {

    vkResetCommandBuffer(mCommandBuffers_[frameIndex][name], 0);

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(mCommandBuffers_[frameIndex][name], &cmdBeginInfo) != VK_SUCCESS) return CMD_BUF_BEGIN_ERR;

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = mRenderPass_;
    renderPassBeginInfo.framebuffer = mFramebuffers_[mCurrentImageIndex_];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = mSwapchainExtent_;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(mCommandBuffers_[frameIndex][name], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::AddToCommand(std::string cmdName, std::string pipelineName, std::string bufferName, u32_t vertexCount, u32_t instanceCount) noexcept {

    vkCmdBindPipeline(mCommandBuffers_[frameIndex][cmdName], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipelines_[pipelineName]);

    vkCmdPushConstants(mCommandBuffers_[frameIndex][cmdName], mPipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VkExtent2D), &mSwapchainExtent_);

    if (!bufferName.empty()) {
        vkCmdBindDescriptorSets(mCommandBuffers_[frameIndex][cmdName], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout_, 0, 1, &mDescriptorSets_[frameIndex][bufferName], 0, nullptr);
    }

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapchainExtent_.width);
    viewport.height = static_cast<float>(mSwapchainExtent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCommandBuffers_[frameIndex][cmdName], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mSwapchainExtent_;
    vkCmdSetScissor(mCommandBuffers_[frameIndex][cmdName], 0, 1, &scissor);
    
    vkCmdDraw(mCommandBuffers_[frameIndex][cmdName], vertexCount, instanceCount, 0, 0);

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::ExecuteCommand(std::string name) noexcept {

    vkCmdEndRenderPass(mCommandBuffers_[frameIndex][name]);

    if (vkEndCommandBuffer(mCommandBuffers_[frameIndex][name]) != VK_SUCCESS) return CMD_BUF_END_ERR;

    mExecuteCommandBuffers_.push_back(name);

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::CreateDataBuffer(std::string name, size_t size) noexcept {

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        MemBuffer &buffer = mMemBuffers_[i][name];
 
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mLogicalDevice_, &bufferCreateInfo, nullptr, &buffer.Buffer) != VK_SUCCESS) return DATA_BUF_CREATE_ERR;

        VkMemoryRequirements memoryRequirements{};
        vkGetBufferMemoryRequirements(mLogicalDevice_, buffer.Buffer, &memoryRequirements);

        buffer.AllocSize = memoryRequirements.size;

        u32_t memTypeIndex;
        if (Result res = getMemTypeIndex(mDeviceMemProperties_, memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memTypeIndex); res != Result::LFS_SUCCESS) return res;

        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = buffer.AllocSize;
        allocateInfo.memoryTypeIndex = memTypeIndex;

        if (vkAllocateMemory(mLogicalDevice_, &allocateInfo, nullptr, &buffer.Memory) != VK_SUCCESS) return LFS_ERROR;

        if (vkBindBufferMemory(mLogicalDevice_, buffer.Buffer, buffer.Memory, 0) != VK_SUCCESS) return LFS_ERROR;

        if (vkMapMemory(mLogicalDevice_, buffer.Memory, 0, buffer.AllocSize, 0, &buffer.MemPtr) != VK_SUCCESS) return LFS_ERROR;

        if (!mDescriptorSets_[i].contains(name)) {
            VkDescriptorSetAllocateInfo descrSetAllocInfo{};
            descrSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descrSetAllocInfo.descriptorPool = mDescriptorPool_;
            descrSetAllocInfo.descriptorSetCount = 1;
            descrSetAllocInfo.pSetLayouts = &mDescriptorSetLayout_;

            if (vkAllocateDescriptorSets(mLogicalDevice_, &descrSetAllocInfo, &mDescriptorSets_[i][name]) != VK_SUCCESS) return LFS_ERROR;
        }

        VkDescriptorSet &descrSet = mDescriptorSets_[i][name];

        VkDescriptorBufferInfo bufferDescriptor{};
        bufferDescriptor.buffer = buffer.Buffer;
        bufferDescriptor.offset = 0;
        bufferDescriptor.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writeDescriptor{};
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet = descrSet;
        writeDescriptor.dstBinding = 0;
        writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.pBufferInfo = &bufferDescriptor;

        vkUpdateDescriptorSets(mLogicalDevice_, 1, &writeDescriptor, 0, nullptr);
    }

    return Result::LFS_SUCCESS;
}



void VulkanHandle::DestroyDataBuffer(std::string name) noexcept {
    vkDeviceWaitIdle(mLogicalDevice_);
    if (!mMemBuffers_[0].contains(name)) return;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(mLogicalDevice_, mMemBuffers_[i][name].Buffer, nullptr);
        vkFreeMemory(mLogicalDevice_, mMemBuffers_[i][name].Memory, nullptr);
    }
}



Result VulkanHandle::WriteToBuffer(std::string name, void *data, size_t size) noexcept {

    memcpy(mMemBuffers_[frameIndex][name].MemPtr, data, size);

    return Result::LFS_SUCCESS;
}

size_t VulkanHandle::GetDataBufferSize(std::string name) noexcept {
    if (mMemBuffers_[0].contains(name)) {
        return mMemBuffers_[0][name].AllocSize;
    } else {
        return 0;
    }
}



void VulkanHandle::StartFrame() noexcept {
    vkWaitForFences(mLogicalDevice_, 1, &mInFlightFences_[frameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(mLogicalDevice_, 1, &mInFlightFences_[frameIndex]);
    vkAcquireNextImageKHR(mLogicalDevice_, mSwapchain_, UINT64_MAX, mImageAvailableSemaphores_[frameIndex], VK_NULL_HANDLE, &mCurrentImageIndex_);

}



void VulkanHandle::EndFrame() noexcept {

    std::vector<VkCommandBuffer> commandBuffers(mExecuteCommandBuffers_.size());

    for (size_t i = 0; i < mExecuteCommandBuffers_.size(); i++) {
        commandBuffers[i] = mCommandBuffers_[frameIndex][mExecuteCommandBuffers_[i]];
    }

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores_[frameIndex]};
    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores_[mCurrentImageIndex_]};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = mExecuteCommandBuffers_.size();
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(mQueues_.graphicsQueue, 1, &submitInfo, mInFlightFences_[frameIndex]);

    VkSwapchainKHR swapChains[] = {mSwapchain_};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &mCurrentImageIndex_;

    vkQueuePresentKHR(mQueues_.presentQueue, &presentInfo);


    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    mExecuteCommandBuffers_.clear();
}



Result VulkanHandle::Resized() noexcept {
    vkDeviceWaitIdle(mLogicalDevice_);

    for (auto framebuffer : mFramebuffers_) {
        vkDestroyFramebuffer(mLogicalDevice_, framebuffer, nullptr);
    }

    for (VkImageView &imageView : mSwapchainImageViews_) {
        vkDestroyImageView(mLogicalDevice_, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mLogicalDevice_, mSwapchain_, nullptr);


    if (Result res = mCreateSwapchain_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan swap chain recreated [" << mSwapchainExtent_.width << ", " << mSwapchainExtent_.height << "]" << std::endl;)

    if (Result res = mCreateFrameBuffers_(); res != Result::LFS_SUCCESS) return res;
    NO_ERR_DBG_BLOCK(std::cout << "--> Vulkan frame buffers recreated" << std::endl;)

    return Result::LFS_SUCCESS;
}



Result VulkanHandle::Quit() noexcept {
    return mDestroy_();
}

