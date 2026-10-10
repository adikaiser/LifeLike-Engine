////////////////////////////////////////////////////////////////////////
///
/// -- LifeLike Engine --
/// 
/// MIT License
/// 
/// Copyright (c) 2025 Jonathan Goth
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
////////////////////////////////////////////////////////////////////////

// Header include guard
#ifndef VULKAN_HANDLE_HPP
#define VULKAN_HANDLE_HPP

/// 
/// @file vulkan_handle.hpp
///
/// @brief VulkanHandle
/// 
/// Everything needed for 
/// instantiating and handling
/// Vulkan for an SDL window.
///

////////////////////////////
/// Headers
////////////////////////////
#include "types.hpp"
#include "result.hpp"
#include <vulkan/vulkan_core.h>
#include <SDL3/SDL.h>

#include <SDL3/SDL_video.h>
#include <string>
#include <vector>
#include <ostream>
#include <set>
#include <optional>
#include <unordered_map>

namespace lle {

    struct Version {
    private:
        const u16_t mMajor_;
        const u16_t mMinor_;
        const u16_t mPatch_;

    public:

        /// @brief Deleted default constructor
        /// 
        /// Version is designed to be immutable 
        /// => values have to be given at initialization
        Version() = delete;

        constexpr Version(const u16_t major, const u16_t minor, const u16_t patch);

        constexpr u16_t Major() const noexcept;
        constexpr u16_t Minor() const noexcept;
        constexpr u16_t Patch() const noexcept;

        constexpr u32_t ToVulkan() const noexcept;

        friend std::ostream& operator<<(std::ostream &os, const Version inst);
    };

    struct QueueCollection {
        VkQueue graphicsQueue;
        //VkQueue computeQueue;
        VkQueue presentQueue;
        
        std::set<u32_t> indeces;

        std::optional<const u32_t*> graphicsIndex;
        //std::optional<const u32_t*> computeIndex;
        std::optional<const u32_t*> presentIndex;

        b8_t isComplete();
    };

    struct MemBuffer {
        VkBuffer Buffer;
        VkDeviceMemory Memory;
        VkDeviceSize AllocSize;
        void *MemPtr;
    };

    class VulkanHandle {
        static const b8_t sEnableValidation_;
        static i32_t sPicDefaultGPU_(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features);
        static const std::vector<cstr_t> sRequiredValidationLayers_;
        static const std::unordered_map<std::string, std::vector<cstr_t>> sRequiredVkInstExtensions_;
        static const std::vector<cstr_t> sRequiredVkDevExtensions_;
        SDL_Window **mSDL3window_;
        VkInstance mVkInst_;
        std::vector<cstr_t> mInstanceExtensions_;
        cstr_t mAppName_;
        Version mVersion_;
        std::vector<VkPhysicalDevice> mPhysicalDevices_;
        VkPhysicalDevice *mPhysicalDevice_;
        VkPhysicalDeviceMemoryProperties mDeviceMemProperties_;
        VkDevice mLogicalDevice_;
        std::vector<cstr_t> mDeviceExtensions_;
        QueueCollection mQueues_;
        VkSurfaceKHR mVkSurface_;
        VkSwapchainKHR mSwapchain_;
        VkFormat mSwapchainImageFormat_;
        VkExtent2D mSwapchainExtent_;
        std::vector<VkImage> mSwapchainImages_;
        std::vector<VkImageView> mSwapchainImageViews_;
        VkRenderPass mRenderPass_;
        std::vector<VkFramebuffer> mFramebuffers_;
        VkCommandPool mCommandPool_;
        std::vector<std::unordered_map<std::string, VkCommandBuffer>> mCommandBuffers_;
        VkPipelineLayout mPipelineLayout_;
        std::unordered_map<std::string, VkPipeline> mGraphicsPipelines_;
        std::vector<VkSemaphore> mImageAvailableSemaphores_;
        std::vector<VkSemaphore> mRenderFinishedSemaphores_;
        std::vector<VkFence> mInFlightFences_;
        u32_t mCurrentImageIndex_;
        std::vector<std::string> mExecuteCommandBuffers_;
        VkDescriptorPool mDescriptorPool_;
        VkDescriptorSetLayout mDescriptorSetLayout_;
        std::vector<std::unordered_map<std::string, MemBuffer>> mMemBuffers_;
        std::vector<std::unordered_map<std::string, VkDescriptorSet>> mDescriptorSets_;

        Result mCreateInst_() noexcept;
        Result mCheckValidationLayers_() noexcept;
        Result mSetupVulkan_() noexcept;
        Result mCreateSurface_() noexcept;
        Result mListGPUs_() noexcept;
        Result mChooseGPU_(i32_t (*function)(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features)) noexcept;
        Result mCheckQueueFamilySupport_() noexcept;
        Result mCreateLogicalDevice_() noexcept;
        Result mCreateQueues_() noexcept;
        Result mCreateSwapchain_() noexcept;
        Result mCreateImageViews_() noexcept;
        Result mCreateRenderPass_() noexcept;
        Result mCreateFrameBuffers_() noexcept;
        Result mCreateCommandPool_() noexcept;
        Result mCreateDescriptorPool_() noexcept;
        Result mCreateDescriptorLayout_() noexcept;
        Result mCreateSyncObjects_() noexcept;
        Result mCreatePipelineLayout_() noexcept;
        Result mCreateGraphicsPipeline_(std::string name, cstr_t vertexShaderPath, cstr_t fragmentShaderPath) noexcept;

        Result mDestroy_() noexcept;

    public:

        VulkanHandle();

        VulkanHandle(const cstr_t appName, SDL_Window** window);

        ~VulkanHandle();

        Result AutoInit() noexcept;

        Result ChooseGPU(const i32_t deviceIndex) noexcept;

        Result ChooseGPU(i32_t (*function)(const VkPhysicalDeviceProperties &properties, const VkPhysicalDeviceFeatures &features) = sPicDefaultGPU_) noexcept;

        Result CreatePipeline(std::string name, cstr_t vertexShaderPath, cstr_t fragmentShaderPath) noexcept;

        Result CreateCommand(std::string name) noexcept;
        Result DestroyCommand(std::string name) noexcept;
        Result RestartCommand(std::string name) noexcept;
        Result AddToCommand(std::string commandName, std::string pipelineName, std::string bufferName, u32_t vertexCount, u32_t instanceCount) noexcept;
        Result ExecuteCommand(std::string name) noexcept;

        Result CreateDataBuffer(std::string name, size_t size) noexcept;
        void DestroyDataBuffer(std::string name) noexcept;
        Result WriteToBuffer(std::string name, void *data, size_t size) noexcept;
        size_t GetDataBufferSize(std::string name) noexcept;

        void StartFrame() noexcept;
        void EndFrame() noexcept;

        Result Resized() noexcept;

        Result Quit() noexcept;

    };
}

#include "vulkan_handle.inl"

#endif // !VULKAN_HANDLE_HPP
