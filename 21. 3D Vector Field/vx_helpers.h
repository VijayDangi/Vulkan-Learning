#ifndef __VULKAN_HELPER_H__
#define __VULKAN_HELPER_H__

#include <string>
#include "Common.h"
#include "vx_types.h"

namespace VX_FRAMEWORK_NAMESPACE
{
    namespace helper
    {
        const char* GetVulkanErrorCodeString(VkResult errorCode);

        std::string GetVkSurfaceTransformsFlagString(uint32_t bits);

        std::string GetVkCompositeAlphaFlagString(uint32_t bits);

        std::string GetVkImageUsageFlagsString(uint32_t bits);

        const char* GetVkFormatString(uint32_t value);

        const char* GetVkColorSpaceString(uint32_t value);

        const char* GetVkPresentModeString(uint32_t value);

        const char* GetVkObjectTypeName(VkObjectType type);

        bool HasStencilComponent(VkFormat Format);

        VkFormat FindSupportedFormat(VkPhysicalDevice device, const VkFormat *candidates, size_t candidate_count, VkImageTiling tiling, VkFormatFeatureFlags features);

        void LogVulkanPhysicalDevicesInfo(VkInstance vkInstance);

        void LogPhysicalDeviceInfo(VkPhysicalDevice device);

        void LogSwapChainSupportDetails(VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails& details, const char *deviceName, VkPhysicalDeviceType deviceType);
    }
}
#endif // __VULKAN_HELPER_H__
