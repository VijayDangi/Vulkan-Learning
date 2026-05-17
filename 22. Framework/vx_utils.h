#pragma once

#include "Common.h"
#include "vx_helpers.h"
#include "vx_types.h"
#include "texture_internal.h"

namespace VX_FRAMEWORK_NAMESPACE
{
    namespace utils
    {
        /**
         * @brief Instance and Surface APIs
         *              Initialize the Vulkan Library by creating an instance.
         *              The instance is the connection between our application and the vulkan library and 
         *              creating it involves specifying some details about our application to the driver.
         */
        bool CreateVulkanInstance(const char * const *vulkanRequiredLayers, uint32_t numLayers, const char * const *vulkanRequiredExtensions, uint32_t numExtensions, bool enableValidationLayer, VkInstance* pInstance);
        const char** GetVulkanInstanceSupportedLayers(uint32_t *pNumLayers);
        const char** GetVulkanInstanceSupportedExtensions(uint32_t *pNumExtensions);
        uint32_t FindMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void DestroyVulkanInstance(VkInstance *instance);
        bool CreateVulkanSurface(VkInstance vkInstance, HWND windowHandle, VkSurfaceKHR *pSurface);
        VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool GetVulkanPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR surface, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkPhysicalDevice *pPhysicalDevice, VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices *pQueueIndices);
        bool CreateVulkanLogicalDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pDeviceFeatures, VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices& queueIndices, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkDevice *pDevice);

        /**
         * @brief Shader API
        */
        VkShaderModule CreateVkShaderModuleFromFile(VkDevice vkDevice, const std::string& filename);
        VkShaderModule CreateVkShaderModuleFromSource(VkDevice vkDevice, const std::vector<char>& shader_code);

        /**
         * @brief Buffer APIs
         */
        bool CreateVkBuffer(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t *queueFamilies, uint32_t numQueueFamily, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyVkBuffer(VkDevice vkDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);

        /**
         * @brief CreateVkCommandPool()
         */
        bool CreateVkCommandPool(VkDevice vkDevice, VkCommandPoolCreateFlags poolCreateFlag, uint32_t queueFamilyIndex, VkCommandPool *out_commandPool);

        /**
         * @brief CreateVkCommandBuffers()
         */
        bool CreateVkCommandBuffers(VkDevice vkDevice, VkCommandPool commandPool, uint32_t bufferCount, VkCommandBuffer *pCommandBuffers);

        /**
         * @brief CreateVkFramebuffer()
        */
        bool CreateVkFramebuffer(VkDevice device, VkRenderPass renderPass, uint32_t attachment_count, VkImageView *pAttachments, uint32_t width, uint32_t height, uint32_t layers, VkFramebuffer *pFramebuffer);

        /**
         * @brief Image APIs
        */
        bool CreateTexture2D(const char *pFileName, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutDeviceMemory, bool flipTexture, bool generateMipMap);
        bool CreateTextureCubeMapFromEquirectangular(const char *pFileName, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutDeviceMemory, bool flipTexture, bool generateMipMap);
        bool ImageMemoryBarrier(VkCommandBuffer cmdBuf, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layoutCount, uint32_t mipBaseLevel, uint32_t mipLevelCount);

        // Should add transfer src and dst usage flags when creating the image if generating mipmap. Because we need to copy data from staging buffer to image and also copy data from one mip level to another mip level.
        bool CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, int imageWidth, int imageHeight, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlagBits propertyFlags, VkSampleCountFlagBits numSamples, bool isCubemap, VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutMemory);

        bool CopyBufferToImage(VkCommandBuffer commandBuffer, VkImage image, VkBuffer buffer, int imageWidth, int imageHeight, uint32_t layerCount, uint32_t mipLevel);
        
        // Image Level 0 will be in transfer dst layout after this function. If not generating mipmap then it should be required layout.
        bool CopyTextureDataToImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, VkImageAspectFlags aspectFlags, VkImageCreateInfo& imageInfo, VkImage& image, bool isCubemap, const void *pPixels);

        // It is expected that the image layout of all mip levels is 'VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL' before calling this function. After this function, all mip levels should be transition to required layout.
        bool GenerateMipmaps( VkDevice device, VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layerCount);

        bool CreateVkImageViews(VkDevice device, VkImage image, VkImageViewType viewtype, VkFormat format, VkComponentMapping componentMapping, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkImageView& imageView);
    }
}
