#ifndef __VULKAN_HELPER_H__
#define __VULKAN_HELPER_H__

#include <string>

namespace VulkanHelper
{
    const char* GetVulkanErrorCodeString(VkResult errorCode);

    std::string GetVkSurfaceTransformsFlagString(uint32_t bits);

    std::string GetVkCompositeAlphaFlagString(uint32_t bits);

    std::string GetVkImageUsageFlagsString(uint32_t bits);

    const char* GetVkFormatString(uint32_t value);

    const char* GetVkColorSpaceString(uint32_t value);

    const char* GetVkPresentModeString(uint32_t value);

    const char* GetVkObjectTypeName(VkObjectType type);
} // namespace VulkanHelper

#endif // __VULKAN_HELPER_H__