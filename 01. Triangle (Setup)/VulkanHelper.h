#ifndef __VULKAN_HELPER_H__
#define __VULKAN_HELPER_H__

#include <string>

namespace VulkanHelper
{
    const char* GetVulkanErrorCodeString(VkResult errorCode);

    std::string GetVkSurfaceTransformsFlagString(uint32_t bits);

    std::string GetVkCompositeAlphaFlagString(uint32_t bits);

    std::string GetVkImageUsageFlagsString(uint32_t bits);

    std::string GetVkFormatString(uint32_t value);

    std::string GetVkColorSpaceString(uint32_t value);

    std::string GetVkPresentModeString(uint32_t value);
} // namespace VulkanHelper

#endif // __VULKAN_HELPER_H__