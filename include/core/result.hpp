#ifndef RESULT_HPP
#define RESULT_HPP

#include "types.hpp"
#include <iostream>

#if (DEBUG == true)
    #define NO_ERR_DBG_BLOCK(code) code
#else
    #define NO_ERR_DBG_BLOCK(code)
#endif

namespace lle {

    /// 
    /// @brief Result type
    ///
    /// Used to indicate wether a 
    /// function has successfully 
    /// executed or an error has 
    /// occured
    enum Result {
        LFS_SUCCESS = 0,
        LFS_ERROR,

        /// Vulkan instance
        PLATFORM_NO_SUPPORT,
        INST_EXT_NO_SUPPORT,
        VAL_LAYER_NO_SUPPORT,
        INST_CREATE_ERR,
        INST_NOT_DEFINED,

        /// Vulkan surface
        SURFACE_CREATE_ERR,
        
        /// Vulkan physical device
        NO_GPUS_FOUND,
        NO_GPU_SUITABLE,
        GPU_NOT_DEFINED,

        /// Vulkan queues
        QUEUE_FAMILIES_NOT_SUFFICIENT,
        
        /// Vulkan logical device
        DEV_EXT_NO_SUPPORT,
        DEV_CREATE_ERR,

        /// Vulkan swap chain
        NO_SWAPCHAIN_FORMATS,
        NO_SWAPCHAIN_PRESENT_MODES,
        SWAPCHAIN_CREATE_ERR,

        /// Vulkan image views
        IMG_VIEW_CREATE_ERR,

        /// Vulkan render pass
        RENDER_PASS_CREATE_ERR,

        /// Vulkan frame buffers
        FRAME_BUF_CREATE_ERR,

        /// Vulkan command buffers
        CMD_POOL_CREATE_ERR,
        CMD_BUF_ALLOC_ERR,
        CMD_BUF_BEGIN_ERR,
        CMD_BUF_END_ERR,

        /// Vulkan sync objects
        SEMAPHORE_CREATE_ERR,
        FENCE_CREATE_ERR,

        /// Vulkan pipeline
        PIPELINE_LAYOUT_CREATE_ERR,
        SHADER_NO_LOAD,
        SHADER_CREATE_ERR,
        PIPELINE_CREATE_ERR,

        /// Vulkan data buffers
        DATA_BUF_CREATE_ERR,
    };


    constexpr
    Result ResultEvaluation(Result res) {
        switch (res) {
            case LFS_ERROR:
                std::cerr << "-X> Unspecified error occured\n";
                break;
            case PLATFORM_NO_SUPPORT:
                std::cerr << "-X> Current platform is not supported\n";
                break;
            case INST_EXT_NO_SUPPORT:
                std::cerr << "-X> Vulkan required instance extensions not supported\n";
                break;
            case VAL_LAYER_NO_SUPPORT:
                std::cerr << "-X> Vulkan required validation layer not supported\n";
                break;
            case INST_CREATE_ERR:
                std::cerr << "-X> Vulkan instance creation failed\n";
                break;
            case INST_NOT_DEFINED:
                std::cerr << "-X> Vulkan instance is not defined or created\n";
                break;
            case SURFACE_CREATE_ERR:
                std::cerr << "-X> Vulkan surface creation failed\n";
                break;
            case NO_GPUS_FOUND:
                std::cerr << "-X> Vulkan no GPUs found\n";
                break;
            case NO_GPU_SUITABLE:
                std::cerr << "-X> Vulkan none of the GPUs is suiatable\n";
                break;
            case GPU_NOT_DEFINED:
                std::cerr << "-X> Vulkan no GPU defined/chosen\n";
                break;
            case QUEUE_FAMILIES_NOT_SUFFICIENT:
                std::cerr << "-X> Vulkan selected GPU doesn't suit queue family requirements\n";
                break;
            case DEV_EXT_NO_SUPPORT:
                std::cerr << "-X> Vulkan GPU doesn't support required extensions\n";
                break;
            case DEV_CREATE_ERR:
                std::cerr << "-X> Vulkan logical device creation failed\n";
                break;
            case NO_SWAPCHAIN_FORMATS:
                std::cerr << "-X> Vulkan no surface formats for the selected GPU found\n";
                break;
            case NO_SWAPCHAIN_PRESENT_MODES:
                std::cerr << "-X> Vulkan no present modes for the selected GPU found\n";
                break;
            case SWAPCHAIN_CREATE_ERR:
                std::cerr << "-X> Vulkan swap chain creation failed\n";
                break;
            case IMG_VIEW_CREATE_ERR:
                std::cerr << "-X> Vulkan image view creation failed\n";
                break;
            case RENDER_PASS_CREATE_ERR:
                std::cerr << "-X> Vulkan render pass creation failed\n";
                break;
            case FRAME_BUF_CREATE_ERR:
                std::cerr << "-X> Vulkan frame buffer creation failed\n";
                break;
            case CMD_POOL_CREATE_ERR:
                std::cerr << "-X> Vulkan command pool creation failed\n";
                break;
            case CMD_BUF_ALLOC_ERR:
                std::cerr << "-X> Vulkan command buffer allocation failed\n";
                break;
            case CMD_BUF_BEGIN_ERR:
                std::cerr << "-X> Vulkan command buffer beginning failed\n";
                break;
            case CMD_BUF_END_ERR:
                std::cerr << "-X> Vulkan command buffer endin failed\n";
                break;
            case SEMAPHORE_CREATE_ERR:
                std::cerr << "-X> Vulkan semaphore creation failed\n";
                break;
            case FENCE_CREATE_ERR:
                std::cerr << "-X> Vulkan fence creation failed\n";
                break;
            case PIPELINE_LAYOUT_CREATE_ERR:
                std::cerr << "-X> Vulkan pipeline layout creation failed\n";
                break;
            case SHADER_NO_LOAD:
                std::cerr << "-X> Vulkan shader can't be loaded\n";
                break;
            case SHADER_CREATE_ERR:
                std::cerr << "-X> Vulkan shader creation failed\n";
                break;
            case PIPELINE_CREATE_ERR:
                std::cerr << "-X> Vulkan graphics pipeline creation failed\n";
                break;
            case DATA_BUF_CREATE_ERR:
                std::cerr << "-X> Vulkan data buffer creation failed\n";
                break;
            case LFS_SUCCESS:
                NO_ERR_DBG_BLOCK(std::cout << "--> Function ran successfully\n";)
                break;
            default:
                std::cerr << "-X> Result not documented\n";
                break;
        }

        return res;
    }

} // namespace lle 

#endif // !RESULT_HPP
