#pragma once

#include "Common.h"
#include "VulkanHelper.h"
#include "Types.h"

namespace VkUtil
{
    /**
     * @brief CreateVulkanInstance() :
     *              Initialize the Vulkan Library by creating an instance.
     *              The instance is the connection between our application and the vulkan library and 
     *              creating it involves specifying some details about our application to the driver.
     */
    bool CreateVulkanInstance(const char * const *vulkanRequiredLayers, uint32_t numLayers, const char * const *vulkanRequiredExtensions, uint32_t numExtensions, bool enableValidationLayer, VkInstance* pInstance);

    /**
     * @brief GetVulkanInstanceSupportedLayers()
     */
    const char** GetVulkanInstanceSupportedLayers(uint32_t *pNumLayers);

    /**
     * @brief GetVulkanInstanceSupportedExtensions()
     */
    const char** GetVulkanInstanceSupportedExtensions(uint32_t *pNumExtensions);

    /**
     * @brief FindVulkanMemoryType()
     */
    uint32_t FindMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
     * @brief DestroyVulkanInstance()
     */
    void DestroyVulkanInstance(VkInstance *instance);

    /**
     * @brief CreateVulkanSurface()
     */
    bool CreateVulkanSurface(VkInstance vkInstance, HWND windowHandle, VkSurfaceKHR *pSurface);

    /**
     * @brief CreateVkShaderModuleFromFile()
    */
    VkShaderModule CreateVkShaderModuleFromFile(VkDevice vkDevice, const std::string& filename);

    /**
     * @brief CreateVkShaderModuleFromSource()
    */
    VkShaderModule CreateVkShaderModuleFromSource(VkDevice vkDevice, const std::vector<char>& shader_code);

    /**
     * @brief CreateVkBuffer()
     */
    bool CreateVkBuffer(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t *queueFamilies, uint32_t numQueueFamily, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    /**
     * @brief CopyVkBuffer()
     */
    void CopyVkBuffer(VkDevice vkDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);

    /**
     * @brief CreateVkCommandPool()
     */
    bool CreateVkCommandPool(VkDevice vkDevice, VkCommandPoolCreateFlags poolCreateFlag, uint32_t queueFamilyIndex, VkCommandPool *out_commandPool);

    /**
     * @brief CreateVkCommandBuffers()
     */
    bool CreateVkCommandBuffers(VkDevice vkDevice, VkCommandPool commandPool, uint32_t bufferCount, VkCommandBuffer *pCommandBuffers);
};
