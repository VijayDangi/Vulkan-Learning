#pragma once

#include "Common.h"
#include <vector>
#include <array>

    // Type Declaration
#define INVALID_QUEUE_FAMILY_HANDLE -1

namespace VkUtil
{
    namespace Types
    {
        struct QueueFamilyIndices
        {
            uint32_t graphicsFamily = INVALID_QUEUE_FAMILY_HANDLE;
            uint32_t presentFamily = INVALID_QUEUE_FAMILY_HANDLE;       // Queue-specific feature which is Presenting to the Surface.
            uint32_t transferFamily = INVALID_QUEUE_FAMILY_HANDLE;

            bool IsComplete()
            {
                // code
                return graphicsFamily != INVALID_QUEUE_FAMILY_HANDLE &&
                    presentFamily != INVALID_QUEUE_FAMILY_HANDLE &&
                    transferFamily != INVALID_QUEUE_FAMILY_HANDLE;
            }
        };

        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct VulkanBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
        };

        struct VulkanUniformBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            void *mapped{nullptr};
        };
    }
}
