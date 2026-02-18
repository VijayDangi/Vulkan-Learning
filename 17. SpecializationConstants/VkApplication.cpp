#include "Common.h"
// #include <Windows.h>

// #define VK_USE_PLATFORM_WIN32_KHR   // Used to include <vulkan/vulkan_win32.h>
// #include <vulkan/vulkan.h>

#include <vector>
#include <array>
#include <set>
#include <limits>       // std::numeric_limits

#include <string>

#include <chrono>

#include "VkApplication.h"
#include "VkUtil.h"

// #pragma warning(disable: 26451)
#define CGLTF_IMPLEMENTATION
#include "../Common/cgltf/cgltf.h"

#include <ktxvulkan.h>

#define VK_ENABLE_VALIDATION_LAYER 1

#ifndef MIN
    #define MIN(a, b) ( (a) > (b) ? (b) : (a))
#endif

#ifndef MAX
    #define MAX(a, b) ( (a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
    #define CLAMP(_value, _min, _max)  MIN( MAX((_value), (_min)), (_max))
#endif

#define CHECK_FUNCTION_RETURN(x, value_to_be) \
    if(x != value_to_be)    \
    {   \
        LogError(#x " Failed.");    \
        return false;   \
    }

/*
*   Uniform Alignment Requirement:
*       Scalars have to be aligned by N = 4bytes
*       vec2 must be aligned by 2N = 8bytes
*       vec3 or vec4 must be aligned by 4N = 16bytes
*       A nested structure must be aligned by the base alignment of its members up to a multiple of 16.
*       A mat4 matrix must have the same alignment as a vec4.
* 
* inshort uniform buffer should have size multiple of 16 bytes, such a way that every member initial offset is multiple of 16
*/

enum class VertexComponent { Position, Normal, Texcoord, Color};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
    glm::vec4 color;

    static VkPipelineVertexInputStateCreateInfo             pipelineInputState;
    static std::vector<VkVertexInputAttributeDescription>   vertexInputAttributesDescription;
    static VkVertexInputBindingDescription                  inputBindingDescription;

    static VkVertexInputBindingDescription getInputBindingDescription(uint32_t binding)
    {
        return VkVertexInputBindingDescription({binding, sizeof(struct Vertex), VK_VERTEX_INPUT_RATE_VERTEX});
    }

    static VkVertexInputAttributeDescription getInputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component)
    {
        switch(component)
        {
            case VertexComponent::Position:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });

            case VertexComponent::Normal:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
            
            case VertexComponent::Texcoord:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texcoord) });
            
            case VertexComponent::Color:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color) });

            default:
                return VkVertexInputAttributeDescription({});
        }
    }

    static std::vector<VkVertexInputAttributeDescription> getInputAttributeDescriptions(uint32_t binding, std::vector<VertexComponent> components)
    {
        std::vector<VkVertexInputAttributeDescription> attributesDescription;
        uint32_t location = 0;

        for(VertexComponent component: components)
        {
            attributesDescription.push_back(getInputAttributeDescription(binding, location, component));
            location++;
        }

        return attributesDescription;
    }

    static VkPipelineVertexInputStateCreateInfo* getPipelineVertexInputState(const std::vector<VertexComponent> components)
    {
        inputBindingDescription = getInputBindingDescription(0);
        vertexInputAttributesDescription = getInputAttributeDescriptions(0, components);

        pipelineInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineInputState.vertexBindingDescriptionCount = 1;
        pipelineInputState.pVertexBindingDescriptions = &inputBindingDescription;
        pipelineInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributesDescription.size();
        pipelineInputState.pVertexAttributeDescriptions = vertexInputAttributesDescription.data();

        return &pipelineInputState;
    }
};

VkPipelineVertexInputStateCreateInfo            Vertex::pipelineInputState{};
std::vector<VkVertexInputAttributeDescription>  Vertex::vertexInputAttributesDescription;
VkVertexInputBindingDescription                 Vertex::inputBindingDescription{};

// =======================
struct UniformData
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec4 light_position{ 0.f, 2.f, 8.f, 0.f };
} uniform_data;

namespace VkApplication
{
    // Function
    static void VkSetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    bool HasStencilComponent(VkFormat format);

    // Variable Declaration
    const std::vector<const char*> vulkanRequiredInstanceLayers =
    {
#if VK_ENABLE_VALIDATION_LAYER
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    const std::vector<const char*> vulkanRequiredInstanceExtesion =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if VK_ENABLE_VALIDATION_LAYER
            // To set up callback in the program to handle messages and the associated details, we have to set
            // up a debug messenger with a callback using the "VK_EXT_debug_utils" extension.
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    const std::vector<const char*> vulkanRequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    const int MAX_FRAMES_IN_FLIGHT = 2; // Defines how many frames should be processed concurrently.
    uint32_t currentFrame = 0;  // Track of the current frame.

    // Vulkan Specific Variables
    VkInstance          vkInstance{ VK_NULL_HANDLE };
    VkSurfaceKHR        vulkanSurface{ VK_NULL_HANDLE };
    VkPhysicalDevice    vulkanPhysicalDevice{ VK_NULL_HANDLE }; // This object will be implicitly destroyedd when 'VkInstance' is destroyed.
    VkDevice            vulkanLogicalDevice{ VK_NULL_HANDLE };  // Vulkan Logical Device

    VkUtil::Types::QueueFamilyIndices vulkanSelectedPhysicalDeviceQueueFamily;

    struct
    {
        VkSwapchainKHR              handle{ VK_NULL_HANDLE };
        std::vector<VkImage>        images; // The images were created by the implementation for the swap chain and they will be automatically cleaned up once the swap chain has been destroyed.
        std::vector<VkImageView>    imageViews; // Unlike VkImage, VkImageView were explicitly created by us, so need to destroy them.
        VkFormat                    format;
        VkExtent2D                  extent;
        std::vector<VkFramebuffer>  framebuffers;
        bool framebufferResized = false;

        struct
        {
            VkImage image;
            VkDeviceMemory imageMemory;
            VkImageView imageView;
        } depth_buffer{};

    } swapchain{};


    VkRenderPass vulkanRenderPass{ VK_NULL_HANDLE };

    VkPipelineLayout      vulkanPipelineLayout{ VK_NULL_HANDLE };
    VkDescriptorSetLayout vulkanDescriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorSet       vulkanDescriptorSets[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE }; // Don't need to explicitly cleanup descriptor sets, because it will auto freed when descriptor pool destroyed.

    VkCommandPool vulkanGraphicsCommandPool{ VK_NULL_HANDLE };
    VkCommandPool vulkanTransferCommandPool{ VK_NULL_HANDLE };
    VkCommandBuffer vulkanCommandBuffers[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // Command buffers will be automatically freed when their command pool is destroyed, so we don't need explicit clenup.

    // Synchromization Objects
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // To signal that an image has been aquired from the swapchain and is ready for rendering.
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // To signal that rendering has finished and presentation can happen.
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };                  // To make sure only one frame is rendering at a time.

    std::vector<Vertex> modelVertices;
    std::vector<uint32_t> modelIndices;

    VkUtil::Types::VulkanBuffer vulkanModelVertexBuffer; // Vertex Buffer
    VkUtil::Types::VulkanBuffer vulkanModelIndexBuffer;  // Index Buffer
    VkUtil::Types::VulkanUniformBuffer vulkanUniformBuffers[MAX_FRAMES_IN_FLIGHT];  // Uniform Buffers

    VkImageLayout   vulkanModelImageLayout;
    VkImage         vulkanModelTexture;
    VkDeviceMemory  vulkanModelTextureMemory;
    VkImageView     vulkanModelTextureView;
    VkSampler       vulkanModelTextureSampler;

    VkDescriptorPool vulkanDescriptorPool{ VK_NULL_HANDLE };

    struct Pipelines
    {
        VkPipeline phong    { VK_NULL_HANDLE };
        VkPipeline toon     { VK_NULL_HANDLE };
        VkPipeline textured { VK_NULL_HANDLE };
    } pipelines;

    struct Queues
    {
        VkQueue graphics_queue{ VK_NULL_HANDLE };
        VkQueue present_queue{ VK_NULL_HANDLE };
        VkQueue transfer_queue{ VK_NULL_HANDLE };
    } queues;

// ====================================================== Function Definition Begin ====================================================== //
#pragma region SwapChainSetup

    /**
     * @brief ChooseSwapSurfaceFormat()
     */
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        // code
        for(const VkSurfaceFormatKHR& surfaceFormat : availableFormats)
        {
            if((surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) &&
               (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            {
                return surfaceFormat;
            }
        }

        // If above format not found then just return 1st member from array.
        return availableFormats[0];
    }

    /**
     * @brief ChooseSwapPresentMode()
     */
    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        // code
        for(const VkPresentModeKHR& presentMode : availablePresentModes)
        {
            if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)  // A.K.A. Triple Buffering ( Used when energy usage is not concern.)
            {
                return presentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;    // For Devices Where energy usage is more important.
    }

    /**
     * @brief ChooseSwapExtend()
     */
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        // code
#ifdef max
#undef max  // Doing this as max() macro is define in Windows.h
#endif

        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            RECT rect;
            GetClientRect(ghwnd, &rect);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(rect.right),
                static_cast<uint32_t>(rect.bottom)
            };

            actualExtent.width = CLAMP( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = CLAMP( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    /**
     * @brief CreateVulkanSwapChain()
     * @return 
     */
    static bool CreateVulkanSwapChain()
    {
        // code
        VkUtil::Types::SwapChainSupportDetails swapChainSupport = VkCustomAPI::GetSwapChainSupportDetails(vulkanPhysicalDevice, vulkanSurface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

            // Sticking to minimum means that we may sometimes have to wait on the driver to complete
            // internal operations before we can aquire another image to render to.
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

            // Make sure to not exceed the maximum number of images. Where 0 is a special value that
            // means that there is no maximum.
        if((swapChainSupport.capabilities.maxImageCount > 0) && (imageCount > swapChainSupport.capabilities.maxImageCount))
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

            // Create Swap Chain
        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface          = vulkanSurface;

            // Specify Details images
        swapChainCreateInfo.minImageCount    = imageCount;
        swapChainCreateInfo.imageFormat      = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent      = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            // Specify how to handle swap chain images that will be across multiple queue families.
        uint32_t queueFamilyIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily, vulkanSelectedPhysicalDeviceQueueFamily.presentFamily};

        if(vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily != vulkanSelectedPhysicalDeviceQueueFamily.presentFamily)
        {
            // VK_SHARING_MODE_CONCURRENT :
            //      Images can be used across multiple queue families without explicit ownership transfers.
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

                // Specify in advance between which queue families ownership will be sahred.
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            // VK_SHARING_MODE_EXCLUSIVE :
            //      An image is owned by one queue family at a time and onwership must be explicitly transferred before
            //      using it in another queue family. This option offers the best performace.
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;  // Optional
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;   // Optional
        }

            // Specify that a certain transform should be applied to images in the swap chain if it supported.
        swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;

            // Speficy if the alpha channel should be used for blending with other windows in the window system.
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            // If 'clipped' member set to 'VK_TRUE' then that means that we don't care about the color of pixels that are obscured,
            // for example because another window is in front of them.
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;

        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateSwapchainKHR( vulkanLogicalDevice, &swapChainCreateInfo, nullptr, &swapchain.handle), false);
        LogSuccess("Vulkan: [Success] Vulkan SwapChain Create Successfully.");

        // Retrieving the swap chain images.
        imageCount = 0;
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapchain.handle, &imageCount, nullptr);
        swapchain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapchain.handle, &imageCount, swapchain.images.data());

        LogSuccess("Vulkan: [Success] %u Images Retrieved from Swap Chain.", imageCount);

        // Store the format and extent we've chosen for the swap chain images.
        swapchain.format = surfaceFormat.format;
        swapchain.extent = extent;

        return true;
    }

    /**
     * @brief: CreateVulkanImageViewsForSwapchain()
    */
    static bool CreateVulkanImageViewsForSwapchain()
    {
        // code
        // An image view is quite literally a view into an image. It describes how to access the image and
        // which part of the image to access, e.g. if it should be treated 2D texture depth as a depth texture
        // without any mipmapping levels.

        size_t imageCount = swapchain.images.size();
        swapchain.imageViews.resize(imageCount);

        for(size_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapchain.images[imageIndex];

                // Specify how the image data should be interpreted.
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapchain.format;

                // The components field allows you to swizzle the color channels around.
                //  e.g. you can map all the channels to the red channel for a monochrome texture.
                // VK_COMPONENT_SWIZZLE_IDENTITY - specifies that the component is set to the identity swizzle.
                // VK_COMPONENT_SWIZZLE_ZERO - specifies that the component is set to zero.
                // VK_COMPONENT_SWIZZLE_ONE - specifies that the component is set to either 1 or 1.0
                // VK_COMPONENT_SWIZZLE_R - specifies that the component is set to the value of the R component of the image.
                // VK_COMPONENT_SWIZZLE_G - specifies that the component is set to the value of the G component of the image.
                // VK_COMPONENT_SWIZZLE_B - specifies that the component is set to the value of the B component of the image.
                // VK_COMPONENT_SWIZZLE_A - specifies that the component is set to the value of the A component of the image.
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                // The 'subresourceRange' field describes what the image's purpose is and which part of the image should be accessed.
                // Our images will be used as color targets without any mipmapping levels or multiple layers.
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            // Create Image View.
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateImageView(vulkanLogicalDevice, &imageViewCreateInfo, nullptr, &swapchain.imageViews[imageIndex]), false);
        }

        LogSuccess("Vulkan: [Success] VkImageView created for swap chain images.");

        return true;
    }

    /**
     * @brief CreateVulkanFramebuffersForSwapchain()
    */
    static bool CreateVulkanFramebuffersForSwapchain()
    {
        // code
            // The attachments specified during render pass creation are bound by wrapping them into a 'VkFramebuffer' object.
            // A framebuffer object references all of the 'VkImageView' objects that represent the attachments.
            // The image that we have to use for the attachment depends on which image the swap chain returns when we retrieve
            // one for presentation.
            // That means that we have to create a framebuffer for all of the images in the swap chain and use the one that
            // corresponds to the retrieved image at drawing time.
        swapchain.framebuffers.resize( swapchain.imageViews.size());

        // Iterate through the image views and create framebuffers from them:
        for(size_t i = 0; i < swapchain.imageViews.size(); ++i)
        {
            VkImageView attachments[] = {swapchain.imageViews[i], swapchain.depth_buffer.imageView};
            if(!VkUtil::CreateVkFramebuffer(vulkanLogicalDevice, vulkanRenderPass, _ARRAYSIZE(attachments), attachments, swapchain.extent.width, swapchain.extent.height, 1, &swapchain.framebuffers[i]))
            {
                LogError("Vulkan: [Error] VkUtil::CreateVkFramebuffer() Failed. ");
                return false;
            }
        }

        LogSuccess("Vulkan: [Success] Vulkan Swapchain Frambuffers Created.");

        return true;
    }


    /**
     * @name GetSupportedFormat()
     */
    VkFormat GetSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for(const VkFormat& format : formats)
        {
            VkFormatProperties props{};
            vkGetPhysicalDeviceFormatProperties( vulkanPhysicalDevice, format, &props);

            if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        LogError("GetSupportedFormat() Failed");
        return VK_FORMAT_UNDEFINED;
    }

    VkFormat GetDepthFormat()
    {
        return GetSupportedFormat({
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    /**
     * @name HasStencilComponent()
     */
    bool HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    /**
     * @name CreateDepthResource()
     */
    static bool CreateDepthResource()
    {
        // code
        VkFormat depthFormat = GetDepthFormat();

        CHECK_FUNCTION_RETURN(
            VkUtil::CreateImage2D(
                vulkanPhysicalDevice,
                vulkanLogicalDevice,
                swapchain.extent.width, swapchain.extent.height, 1, depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                &swapchain.depth_buffer.image, &swapchain.depth_buffer.imageMemory
            ),
            true
        );

        CHECK_FUNCTION_RETURN(VkUtil::Create2DImageView( vulkanLogicalDevice, swapchain.depth_buffer.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &swapchain.depth_buffer.imageView), true);

        // Create Separate Command Buffer for texture loading
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
        CHECK_FUNCTION_RETURN( VkUtil::CreateVkCommandBuffers( vulkanLogicalDevice, vulkanGraphicsCommandPool, 1, &cmdBuffer), true);

        // begin command recording
        VkCommandBufferBeginInfo cmdBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
        vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

        VkImageSubresourceRange subResourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        // Image Barrier for optimal image
        // Optimal image wiil be used as destination for the copy
        VkSetImageLayout(cmdBuffer, swapchain.depth_buffer.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, subResourceRange);

        // Flush commands.
        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        VkFence fence{};
        CHECK_FUNCTION_RETURN( vkCreateFence(vulkanLogicalDevice, &fenceInfo, nullptr, &fence), VK_SUCCESS);

        // submit queue
        CHECK_FUNCTION_RETURN(vkQueueSubmit(queues.graphics_queue, 1, &submitInfo, fence), VK_SUCCESS);

        // Wait for the fence to signal that command buffer has finished executing
        CHECK_FUNCTION_RETURN(vkWaitForFences(vulkanLogicalDevice, 1, &fence, VK_TRUE, UINT64_MAX), VK_SUCCESS);

        // cleanp
        vkDestroyFence(vulkanLogicalDevice, fence, nullptr);
        fence = VK_NULL_HANDLE;

        vkFreeCommandBuffers( vulkanLogicalDevice, vulkanGraphicsCommandPool, 1, &cmdBuffer);
        cmdBuffer = nullptr;

        return true;
    }

    #pragma endregion SwapChainSetup

    /**
     * @brief CreateVulkanRenderPass()
     * @return 
     */
    static bool CreateVulkanRenderPass()
    {
        // code
////////////// Attachment Description.
        VkAttachmentDescription colorAttachmentDescrition{};
        colorAttachmentDescrition.format = swapchain.format;
        colorAttachmentDescrition.samples = VK_SAMPLE_COUNT_1_BIT;

        // 'loadOp' and 'storeOp' determine what to do with the data in the attachment before rendering and
        // after rendering.
        //
        // VK_ATTACHMENT_LOAD_OP_LOAD      :- Preserve the existing contents of the attachment.
        // VK_ATTACHMENT_LOAD_OP_CLEAR     :- Clear the values to a constantat the start.
        // VK_ATTACHMENT_LOAD_OP_DONT_CARE :- Existing contents are undefined; we don't care about them.
        colorAttachmentDescrition.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        // VK_ATTACHMENT_STORE_OP_STORE     :- Rendered contents will be stored in memory and can be read later.
        // VK_ATTACHMENT_STORE_OP_DONT_CARE :- Contents of the framebuffer will be undefined after the rendering operation.
        colorAttachmentDescrition.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // 'loadOp', 'storeOp' apply to color and depth data, and
        // 'stencilLoadOp', 'stencilStoreOp' apply to stencil data.
        colorAttachmentDescrition.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescrition.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format,
        // however the layout of the pixels in memory can change based on what you're trying to do with an image.
        //
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :- Images used as color attachment.
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR          :- Images to be presented in the swap chain.
        // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL     :- Images to be used as destination for a memory copy operation.
        colorAttachmentDescrition.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;        // Specifies which layout the image will have before the render pass begins.
        colorAttachmentDescrition.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // Specifies the layout to automatically transition to when the render pass finishes.

////////////// Depth Attachment Description.
        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format         = GetDepthFormat();
        depthAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


////////////// Subpasses and attachment references.
        // A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the
        // contents of framebuffers in previous passes, for example a sequence of post-processing effects that are applied one after another.

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;        // Specifies which attachment to reference by its index in the attachment descriptions array.
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifies which layout we would like the attachment to have during a subpass that uses this reference.

        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

#if 0
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;

        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
#endif

////////////// Render Pass
        VkAttachmentDescription attachments[] = { colorAttachmentDescrition, depthAttachmentDescription};

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = _ARRAYSIZE(attachments);
        renderPassCreateInfo.pAttachments = attachments;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;

#if 0
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
#endif

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateRenderPass( vulkanLogicalDevice, &renderPassCreateInfo, nullptr, &vulkanRenderPass), false);
        LogSuccess("Vulkan: [Success] Vulkan Graphics Render Pass Created.");

        return true;
    }

    /**
     * @brief CreateVulkanGraphicsPipelines()
    */
    static bool CreateVulkanGraphicsPipelines()
    {
        // code
////////////// Pipeline Layout
            // The uniform values need to be specified during pipeline creation by creating VkPipelineLayout object.
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &vulkanDescriptorSetLayout;
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreatePipelineLayout(vulkanLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &vulkanPipelineLayout), false);


////////////// Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

////////////// Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo{};
        rasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;   // if it true then geometry never passes through the rasterizer stage. (Disable any output to the framebuffer).
        rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;   // To use other than fill mode, we have to enable GPU feature.
        rasterizerStateCreateInfo.lineWidth = 1.0f;                     // To use line width grater than 1.0f, we have to enable GPU feature.
        rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizerStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizerStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizerStateCreateInfo.depthBiasConstantFactor = 0.0f;   // Optional
        rasterizerStateCreateInfo.depthBiasClamp = 0.0f;   // Optional
        rasterizerStateCreateInfo.depthBiasSlopeFactor = 0.0f;   // Optional


////////////// Color Blending
        // After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
        // There are two ways of color blending:
        //      1. Mix the old and new value to produce a final color.
        //      2. Combine the old and new value using a bitwise operation.

        /*
            VkPipelineColorBlendAttachmentState is per-framebuffer struct allows to configure the first way of color blending.
            Psuedocode:

            if(blendEnable)
            {
                finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
                finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
            }
            else
            {
                finalColor = newColor;
            }

            finalColor = finalColor & colorWriteMask;
        */
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};    // Contains the configuration per attached framebuffer.
        colorBlendAttachmentState.colorWriteMask        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable           = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor   = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstColorBlendFactor   = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.colorBlendOp          = VK_BLEND_OP_ADD;   // Optional
        colorBlendAttachmentState.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.alphaBlendOp          = VK_BLEND_OP_ADD;   // Optional


        // The VkPipelineColorBlendStateCreateInfo() references the array of structures for all of the framebuffers and
        // allows you to blend constants that you can use as blend factors in the aforementioned calculations.
        //
        // if you want to use the second method of blending (bitwise combination), then you should set 'logicOpEnable'
        // to VK_TRUE.
        // The bitwise operation can then be specified in the 'logicOp' field. Note that this will automatically disable
        // the first method, as if you had set 'blendEnable' to VK_FALSE for every attached framebuffer!
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo{};
        colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlendingCreateInfo.attachmentCount = 1;
        colorBlendingCreateInfo.pAttachments = &colorBlendAttachmentState;

////////////// Depth/Stencil
        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
        depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

////////////// Viewport State
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;

////////////// Multisampling
        // Enabling it requires enabling a GPU feature.
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

////////////// Dynamic State
        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };

            // This will cause the configuration of these values to be ignored and you will be able (and required) to
            // specify the data at drawing time. This results in a more flexible setup and is very common for things
            // like viewport and scissor state, which would result in a more complex setup when being backed into 
            // the pipeline state.
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

////////////// Creating shader modules.
        VkShaderModule vertexShaderModule = VkUtil::CreateVkShaderModuleFromFile(vulkanLogicalDevice, "shaders/vertexShader.vert.spv");
        VkShaderModule fragmentShaderModule = VkUtil::CreateVkShaderModuleFromFile(vulkanLogicalDevice, "shaders/fragmentShader.frag.spv");

        if(!vertexShaderModule || !fragmentShaderModule)
        {
            if(fragmentShaderModule) { vkDestroyShaderModule( vulkanLogicalDevice, fragmentShaderModule, nullptr); }
            if(vertexShaderModule) { vkDestroyShaderModule( vulkanLogicalDevice, vertexShaderModule, nullptr); }

            vertexShaderModule = nullptr;
            fragmentShaderModule = nullptr;
            return false;
        }

////////////// Shader Stage Creation. (Programmable Stage)
        VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
        vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageInfo.module = vertexShaderModule;
        vertexShaderStageInfo.pName = "main";   // Entry-point

        VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
        fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageInfo.module = fragmentShaderModule;
        fragmentShaderStageInfo.pName = "main";   // Entry-point

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

////////////// Create Graphics Pipeline
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

            // Referencing programmable-function stage.
        pipelineCreateInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]);
        pipelineCreateInfo.pStages = shaderStages;

            // Referencing fixed-function stages.
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizerStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.pVertexInputState = Vertex::getPipelineVertexInputState({VertexComponent::Position, VertexComponent::Normal, VertexComponent::Texcoord, VertexComponent::Color });

            // pipeline layout
        pipelineCreateInfo.layout = vulkanPipelineLayout;

            // Finally renference to the render pass and the index of the sub pass where this graphics pipeline will be used.
        pipelineCreateInfo.renderPass = vulkanRenderPass;
        pipelineCreateInfo.subpass = 0;

        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

            // Prepare specialization constants data
        
        // host data to take specialization constants from
        struct SpecializationData
        {
            // Sets the lighting model used in the fragmnet 'uber' shader
            uint32_t lighting_model{ 0 };
            // Parameter for the toon shading part of the fragment shader
            float toon_desaturation_factor{ 0.5f };
        } specializationData;

        // Each shader constant of a shader stage corresponds to one map entry
        VkSpecializationMapEntry specializationMapEntries[2]{};
            // Shader bindings based on specialization constants are marked by the new "constant_id" layout qualifier:
            //  layout (constant_id = 0) const int LIGHTING_MODEL = 0;
            //  layout (constant_id = 1) const float PARAM_TOON_DESATURATION = 0.0f;
        
        // Map entry for the lighting model to be used by the fragment shader
        specializationMapEntries[0].constantID = 0;
        specializationMapEntries[0].size = sizeof(specializationData.lighting_model);
        specializationMapEntries[0].offset = 0;

        // Map entry for the toon shader parameter
        specializationMapEntries[1].constantID = 1;
        specializationMapEntries[1].size = sizeof(specializationData.toon_desaturation_factor);
        specializationMapEntries[1].offset = offsetof(SpecializationData, toon_desaturation_factor);

        // Prepare specialization info block for the shader stage
        VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount = _ARRAYSIZE(specializationMapEntries);
        specializationInfo.pMapEntries = specializationMapEntries;
        specializationInfo.dataSize = sizeof(specializationData);
        specializationInfo.pData = &specializationData;

        shaderStages[1].pSpecializationInfo = &specializationInfo;

            // Create Pipeline
            // All pipeline will use the same shader and specialization constants to change branching and parameters of that shader
        // Solid phong shading
        specializationData.lighting_model = 0;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.phong), false);

        // Phong and textured
        specializationData.lighting_model = 1;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.toon), false);

        // Textured discard
        specializationData.lighting_model = 2;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.textured), false);

        // Destroy Shader Module
        vkDestroyShaderModule( vulkanLogicalDevice, fragmentShaderModule, nullptr);
        vkDestroyShaderModule( vulkanLogicalDevice, vertexShaderModule, nullptr);

        LogSuccess("Vulkan: [Success] Vulkan Graphics Pipeline Created.");

        return true;
    }

    /**
     * @brief CreateVulkanSyncObjects()
     */
    static bool CreateVulkanSyncObjects()
    {
        // code
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            // Create a fence in the signaled state, so that first call to vkWaitForFences() in DrawFrame() returns
            // immediately since the fence is already signaled.
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateSemaphore(vulkanLogicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), false);

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateSemaphore(vulkanLogicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), false);

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateFence(vulkanLogicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]), false);
        }
        
        return true;
    }

    /**
     * @brief CreateVulkanBuffer()
     */
    static bool CreateVulkanBuffer( VkDeviceSize maxBufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkUtil::Types::VulkanBuffer& buffer, void *data = nullptr, size_t dataSizeBytes = 0)
    {
        // Code
        if(maxBufferSize < dataSizeBytes)
        {
            LogError("MaxBufferSize is less than the data size.");
            return false;
        }

        if(dataSizeBytes == 0)
        {
            dataSizeBytes = maxBufferSize;
        }

        uint32_t queueFamiliesIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily};
        uint32_t numQueues = 2;

        // Create required buffer
        if(!VkUtil::CreateVkBuffer( vulkanPhysicalDevice, vulkanLogicalDevice, maxBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, memoryProperties, queueFamiliesIndices, numQueues, buffer.handle, buffer.memory))
        {
            LogError("CreateBuffer() Failed.");
            return false;
        }

        // if there is data then copy it to buffer.
        if(data != nullptr)
        {
            VkUtil::Types::VulkanBuffer stagingBuffer{};
            if(!VkUtil::CreateVkBuffer( vulkanPhysicalDevice, vulkanLogicalDevice, maxBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queueFamiliesIndices, numQueues, stagingBuffer.handle, stagingBuffer.memory))
            {
                LogError("CreateBuffer() Failed.");
                return false;
            }

            // Map Buffer
            void *mapped_ptr = nullptr;
            vkMapMemory(vulkanLogicalDevice, stagingBuffer.memory, 0, maxBufferSize, 0, &mapped_ptr);
                memcpy(mapped_ptr, data, dataSizeBytes);
            vkUnmapMemory(vulkanLogicalDevice, stagingBuffer.memory);
            mapped_ptr = nullptr;

            // Copy Data from staging to buffer
            VkUtil::CopyVkBuffer(vulkanLogicalDevice, vulkanTransferCommandPool, queues.transfer_queue, stagingBuffer.handle, buffer.handle, dataSizeBytes);

            // Cleanup
            if(stagingBuffer.handle)
            {
                vkDestroyBuffer(vulkanLogicalDevice, stagingBuffer.handle, nullptr);
                stagingBuffer.handle = nullptr;
            }

            if(stagingBuffer.memory)
            {
                vkFreeMemory(vulkanLogicalDevice, stagingBuffer.memory, nullptr);
                stagingBuffer.memory = nullptr;
            }
        }

        return true;
    }

    /**
     * @brief CreateUniformBuffer()
     */
    static bool CreateUniformBuffer( VkDeviceSize bufferSize, VkUtil::Types::VulkanUniformBuffer *uniformBuffers, size_t num_buffers)
    {
        // code
        uint32_t queueFamiliesIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily};
        uint32_t numQueues = 2;

        for(size_t i = 0; i < num_buffers; ++i)
        {
            if(!VkUtil::CreateVkBuffer( vulkanPhysicalDevice, vulkanLogicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queueFamiliesIndices, numQueues, uniformBuffers[i].handle, uniformBuffers[i].memory))
            {
                LogError("[Error] CreateBuffer() Failed for Uniform at %d.", (int)i);
                return false;
            }

            vkMapMemory(vulkanLogicalDevice, uniformBuffers[i].memory, 0, bufferSize, 0, &uniformBuffers[i].mapped);
        }

        return true;
    }

    /**
     * @brief CreateDescriptorPool()
     */
    static bool CreateDescriptorPool()
    {
        // code
        VkDescriptorPoolSize poolSizes[2];
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = _ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDescriptorPool(vulkanLogicalDevice, &poolInfo, nullptr, &vulkanDescriptorPool), false);
        return true;
    }

    /**
     * @brief CreateVulkanDescriptorSetLayout()
    */
    static bool CreateVulkanDescriptorSetLayout()
    {
        // code
        VkDescriptorSetLayoutBinding layoutBinding[2]{};
        layoutBinding[0].binding = 0;           // Same value as in vertex shader.
        layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding[0].descriptorCount = 1;
        layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        layoutBinding[0].pImmutableSamplers = nullptr;

        layoutBinding[1].binding = 1;
        layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBinding[1].descriptorCount = 1;
        layoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBinding[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 2;
        layoutInfo.pBindings = layoutBinding;
        
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDescriptorSetLayout( vulkanLogicalDevice, &layoutInfo, nullptr, &vulkanDescriptorSetLayout), false);
        return true;
    }

    /**
     * @brief CreateDescriptorSets()
     */
    static bool CreateDescriptorSets()
    {
        // code
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = vulkanDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &vulkanDescriptorSetLayout;

        for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkAllocateDescriptorSets(vulkanLogicalDevice, &allocInfo, &vulkanDescriptorSets[i]), false);

            // Configure Desciptor
            VkWriteDescriptorSet descriptorWrite[2]{};

                // Uniform
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vulkanUniformBuffers[i].handle;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformData);

            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = vulkanDescriptorSets[i];
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].pImageInfo = nullptr;
            descriptorWrite[0].pTexelBufferView = nullptr;

                // Sampelr
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = vulkanModelImageLayout;
            imageInfo.imageView = vulkanModelTextureView;
            imageInfo.sampler = vulkanModelTextureSampler;

            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = vulkanDescriptorSets[i];
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].pBufferInfo = nullptr;
            descriptorWrite[1].pImageInfo = &imageInfo;
            descriptorWrite[1].pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(vulkanLogicalDevice, 2, descriptorWrite, 0, nullptr);
        }

        return true;
    }

    /**
     * @brief LoadGLTFModel()
     * @param file_path 
     * @return 
     */
    static bool LoadGLTFModel(const char *file_path)
    {
        // code
        cgltf_options options{};

        // Parse glTF file
        cgltf_data *gltfData = nullptr;
        cgltf_result result = cgltf_parse_file(&options, file_path, &gltfData);
        if(result != cgltf_result::cgltf_result_success)
        {
            LogError("Unable to load file \"%s\".", file_path);
            return false;
        }

        // Decode base64 buffer data
        result = cgltf_load_buffers(&options, gltfData, file_path);
        if(result != cgltf_result::cgltf_result_success)
        {
            cgltf_free(gltfData);
            gltfData = nullptr;

            LogError("Failed to load buffer data.");
            return false;
        }

        // Validate glTF file
        result = cgltf_validate(gltfData);
        if(result != cgltf_result::cgltf_result_success)
        {
            cgltf_free(gltfData);
            LogError("Invalid glTF file \"%s\".", file_path);
            return false;
        }

        std::vector<std::vector<glm::vec3>> positions;
        std::vector<std::vector<glm::vec3>> normals;
        std::vector<std::vector<glm::vec4>> colors;
        std::vector<std::vector<glm::vec2>> texcoords;
        std::vector<std::vector<uint32_t>> indices;

        // Traverse Nodes
        for(cgltf_size node_index = 0; node_index < gltfData->nodes_count; ++node_index)
        {
            cgltf_node &node = gltfData->nodes[node_index];
            if(node.mesh == nullptr)
            {
                continue;
            }

            // Traverse Primitives
            for(cgltf_size primitive_index = 0; primitive_index < node.mesh->primitives_count; ++primitive_index)
            {
                size_t currentPrimitive = positions.size();

                positions.resize( currentPrimitive + 1);
                normals.resize( currentPrimitive + 1);
                texcoords.resize( currentPrimitive + 1);
                colors.resize( currentPrimitive + 1);
                indices.resize( currentPrimitive + 1);

                // Traverse Attributes
                cgltf_primitive &primitive = node.mesh->primitives[primitive_index];
                for(cgltf_size attribute_index = 0; attribute_index < primitive.attributes_count; ++attribute_index)
                {
                    cgltf_attribute &attribute = primitive.attributes[attribute_index];

                    cgltf_accessor *accessor = attribute.data;

                    uint8_t component_count = 0;
                    if(accessor->type == cgltf_type_vec2) { component_count = 2; }
                    else if(accessor->type == cgltf_type_vec3) { component_count = 3; }
                    else if(accessor->type == cgltf_type_vec4) { component_count = 4; }

                    // std::vector<float> values(accessor->count * component_count);

                    float values[4]{};
#if 0
                    float yMultiplier = -1;
#else
                    float yMultiplier = 1;
#endif

                    for(cgltf_size i = 0; i < accessor->count; ++i)
                    {
                        cgltf_accessor_read_float(accessor, i, values, component_count);
                        
                        // Fill attribute array
                        switch(attribute.type)
                        {
                            case cgltf_attribute_type_position:
                                positions[currentPrimitive].push_back(glm::vec3(values[0], values[1] * yMultiplier, values[2]));
                            break;

                            case cgltf_attribute_type_texcoord:
                                texcoords[currentPrimitive].push_back(glm::vec2(values[0], values[1]));
                            break;

                            case cgltf_attribute_type_normal:
                            {
                                // after reading normal, check its squared length. If normal is invalid, return a valid vector.
                                glm::vec3 normal = glm::vec3(values[0], values[1], values[2]);
                                float length = glm::length(normal);
                                if((length * length) < 0.000001f)
                                {
                                    normal.x = 0.0f;
                                    normal.y = 1.0f;
                                    normal.z = 0.0f;
                                }
                                normal.y *= yMultiplier;
                                normals[currentPrimitive].push_back(glm::normalize(normal));
                            }
                            break;

                            case cgltf_attribute_type_color:
                            {
                                if(component_count == 1)
                                {
                                    colors[currentPrimitive].push_back( glm::vec4(values[0], values[0], values[0], 1.0f));
                                }
                                else if(component_count == 2)
                                {
                                    colors[currentPrimitive].push_back( glm::vec4(values[0], values[1], 0.0f, 1.0f));
                                }
                                else if(component_count == 3)
                                {
                                    colors[currentPrimitive].push_back( glm::vec4(values[0], values[1], values[2], 1.0f));
                                }
                                else if(component_count == 4)
                                {
                                    colors[currentPrimitive].push_back( glm::vec4(values[0], values[1], values[2], values[3]));
                                }
                            }
                            break;
                        }
                    }
                }

                // Check whether primitive contains indices.
                if(primitive.indices != nullptr)
                {
                    for(cgltf_size i = 0; i < primitive.indices->count; ++i)
                    {
                        indices[currentPrimitive].push_back( cgltf_accessor_read_index(primitive.indices, i));
                    }
                }
            }
        }

        cgltf_free(gltfData);
        gltfData = nullptr;

        uint32_t vertexOffset = 0;
        
        for(int i = 0; i < positions.size(); ++i)
        {
            LogInfo( "\n%d. Position: %d, Normals: %d, Texcoords: %d, Colors: %d, Indices: %d.",
                i, positions[i].size(), normals[i].size(), texcoords[i].size(), colors[i].size(), indices[i].size()
            );

            // Combine Vertice and Indices
            for(int j = 0; j < positions[i].size(); ++j)
            {
                glm::vec3 normal(0.0f, 1.0f, 0.0f);
                if(normals[i].size() != 0)
                {
                    normal = normals[i][j];
                }

                glm::vec2 uv(0.0f, 0.0f);
                if(texcoords[i].size() != 0)
                {
                    uv = texcoords[i][j];
                }

                glm::vec4 color(1.0f);
                if(colors[i].size() != 0)
                {
                    color = colors[i][j];
                }

                modelVertices.push_back(Vertex{ positions[i][j], normal, uv, color});
            }

            for(int index = 0; index < indices[i].size(); ++index)
            {
                modelIndices.push_back(vertexOffset + indices[i][index]);
            }

            vertexOffset += positions[i].size();
        }

        return true;
    }

    /**
     * @brief VkSetImageLayout()
     */
    static void VkSetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
    {
        // code
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldImageLayout;
        barrier.newLayout = newImageLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = subresourceRange;

        // source layout
        switch(oldImageLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined ( or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                barrier.srcAccessMask = 0;
            break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is a read by a shader
                // Make sure any shader reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

            default:
                // Other source layout aren't handled
            break;
        }


        // destination layout
        switch(newImageLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as transfer destination
                // Make sure any writes to the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as transfer source
                // Make sure any reads from the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image will be used as depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be used as read by a shader
                // Make sure any shader reads from the image have been finished
                if(barrier.srcAccessMask == 0)
                {
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

            default:
                // Other source layout aren't handled
            break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            cmdBuffer,
            srcStageMask, dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    /**
     * @brief
     */
    static bool LoadKtxTexture(const char *file_path, VkFormat format, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // code
        ktxTexture *ktxTexture = nullptr;
        CHECK_FUNCTION_RETURN(ktxTexture_CreateFromNamedFile(file_path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture), KTX_SUCCESS);

        ktx_uint32_t width     = ktxTexture->baseWidth;
        ktx_uint32_t height    = ktxTexture->baseHeight;
        ktx_uint32_t mipLevels = ktxTexture->numLevels;

        ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

        // Get device properties for the requested texture format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(vulkanPhysicalDevice, format, &formatProperties);

        // Create Separate Command Buffer for texture loading
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
        CHECK_FUNCTION_RETURN( VkUtil::CreateVkCommandBuffers( vulkanLogicalDevice, vulkanGraphicsCommandPool, 1, &cmdBuffer), true);

        // begin command recording
        VkCommandBufferBeginInfo cmdBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
        vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);

        // Create a host-visible staging buffer that contains the raw image data
        VkUtil::Types::VulkanBuffer stagingBuffer;

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = ktxTextureSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CHECK_FUNCTION_RETURN(vkCreateBuffer( vulkanLogicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer.handle), VK_SUCCESS);

        // get memory type
        VkMemoryRequirements memReq{};
        vkGetBufferMemoryRequirements(vulkanLogicalDevice, stagingBuffer.handle, &memReq);
        
        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReq.size;
        memAllocInfo.memoryTypeIndex = VkUtil::FindMemoryType(vulkanPhysicalDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        CHECK_FUNCTION_RETURN( vkAllocateMemory(vulkanLogicalDevice, &memAllocInfo, nullptr, &stagingBuffer.memory), VK_SUCCESS);
        CHECK_FUNCTION_RETURN( vkBindBufferMemory(vulkanLogicalDevice, stagingBuffer.handle, stagingBuffer.memory, 0), VK_SUCCESS);
        
        // Copy texture data into staging buffer
        uint8_t *data{ nullptr };
        CHECK_FUNCTION_RETURN( vkMapMemory(vulkanLogicalDevice, stagingBuffer.memory, 0, memReq.size, 0, (void**) &data), VK_SUCCESS);
        memcpy( data, ktxTextureData, ktxTextureSize);
        vkUnmapMemory(vulkanLogicalDevice, stagingBuffer.memory);
        data = nullptr;

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;

        for(uint32_t i = 0; i < mipLevels; ++i)
        {
            ktx_size_t offset;
            CHECK_FUNCTION_RETURN( ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset), KTX_SUCCESS);

            VkBufferImageCopy bufferCopyRegion{};
            bufferCopyRegion.bufferOffset = offset;
            
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;

            bufferCopyRegion.imageExtent.width = MAX(1u, ktxTexture->baseWidth >> i);
            bufferCopyRegion.imageExtent.height = MAX(1u, ktxTexture->baseHeight >> i);
            bufferCopyRegion.imageExtent.depth = 1;

            bufferCopyRegions.push_back(bufferCopyRegion);
        }

        // Create optimal tiled target image
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        CHECK_FUNCTION_RETURN(vkCreateImage(vulkanLogicalDevice, &imageCreateInfo, nullptr, &vulkanModelTexture), VK_SUCCESS);

            // memory type
        vkGetImageMemoryRequirements(vulkanLogicalDevice, vulkanModelTexture, &memReq);

        memAllocInfo.allocationSize = memReq.size;
        memAllocInfo.memoryTypeIndex = VkUtil::FindMemoryType(vulkanPhysicalDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CHECK_FUNCTION_RETURN( vkAllocateMemory(vulkanLogicalDevice, &memAllocInfo, nullptr, &vulkanModelTextureMemory), VK_SUCCESS);
        CHECK_FUNCTION_RETURN( vkBindImageMemory(vulkanLogicalDevice, vulkanModelTexture, vulkanModelTextureMemory, 0), VK_SUCCESS);

        VkImageSubresourceRange subResourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };

        // Image Barrier for optimal image
        // Optimal image wiil be used as destination for the copy
        VkSetImageLayout(cmdBuffer, vulkanModelTexture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subResourceRange);

        // Copy mip levels from staging buffer
        vkCmdCopyBufferToImage(
            cmdBuffer,
            stagingBuffer.handle,
            vulkanModelTexture,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            bufferCopyRegions.size(),
            bufferCopyRegions.data()
        );

        VkSetImageLayout(cmdBuffer, vulkanModelTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout, subResourceRange);

        // Flush commands.
        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        VkFence fence{};
        CHECK_FUNCTION_RETURN( vkCreateFence(vulkanLogicalDevice, &fenceInfo, nullptr, &fence), VK_SUCCESS);

        // submit queue
        CHECK_FUNCTION_RETURN(vkQueueSubmit(queues.graphics_queue, 1, &submitInfo, fence), VK_SUCCESS);

        // Wait for the fence to signal that command buffer has finished executing
        CHECK_FUNCTION_RETURN(vkWaitForFences(vulkanLogicalDevice, 1, &fence, VK_TRUE, UINT64_MAX), VK_SUCCESS);


        // Create Sampler
        VkPhysicalDeviceFeatures deviceFeature;
        vkGetPhysicalDeviceFeatures(vulkanPhysicalDevice, &deviceFeature);

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &deviceProperties);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.anisotropyEnable = deviceFeature.samplerAnisotropy;
        samplerCreateInfo.maxAnisotropy = deviceFeature.samplerAnisotropy ? deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = (float)mipLevels;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        CHECK_FUNCTION_RETURN(vkCreateSampler(vulkanLogicalDevice, &samplerCreateInfo, nullptr, &vulkanModelTextureSampler), VK_SUCCESS);

        // Create image view
            // Textures are not directly accessed by the shaders and are abstracted by image views containing additional information and subresource ranges
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = vulkanModelTexture;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = format;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = mipLevels;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        CHECK_FUNCTION_RETURN(vkCreateImageView(vulkanLogicalDevice, &viewCreateInfo, nullptr, &vulkanModelTextureView), VK_SUCCESS);

        // cleanup
        vkDestroyFence(vulkanLogicalDevice, fence, nullptr);
        fence = VK_NULL_HANDLE;

        vkFreeCommandBuffers( vulkanLogicalDevice, vulkanGraphicsCommandPool, 1, &cmdBuffer);
        cmdBuffer = nullptr;

        vkDestroyBuffer(vulkanLogicalDevice, stagingBuffer.handle, nullptr);
        vkFreeMemory(vulkanLogicalDevice, stagingBuffer.memory, nullptr);

        ktxTexture_Destroy(ktxTexture);

        vulkanModelImageLayout = imageLayout;

        return true;
    }


    void RecreatePipeline()
    {
        vkDeviceWaitIdle(vulkanLogicalDevice);

        if(vulkanPipelineLayout)
        {
            vkDestroyPipelineLayout(vulkanLogicalDevice, vulkanPipelineLayout, nullptr);
            vulkanPipelineLayout = nullptr;
        }

        if(pipelines.phong)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.phong, nullptr);
            pipelines.phong = nullptr;
        }

        if(pipelines.textured)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.textured, nullptr);
            pipelines.textured = nullptr;
        }

        if(pipelines.toon)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.toon, nullptr);
            pipelines.toon = nullptr;
        }


        CreateVulkanGraphicsPipelines();
    }

    /**
     * @brief InitializeVulkan()
     */
    bool InitializeVulkan(HWND windowHandle)
    {
        // code
        // Create Vulkan Instance
        CHECK_FUNCTION_RETURN(VkUtil::CreateVulkanInstance(vulkanRequiredInstanceLayers.data(), vulkanRequiredInstanceLayers.size(), vulkanRequiredInstanceExtesion.data(), (uint32_t)vulkanRequiredInstanceExtesion.size(), VK_ENABLE_VALIDATION_LAYER, &vkInstance), true);

        // Create Surface
        CHECK_FUNCTION_RETURN(VkUtil::CreateVulkanSurface(vkInstance, windowHandle, &vulkanSurface), true);

        // Get Physical Device
        VkCustomAPI::LogVulkanPhysicalDevicesInfo(vkInstance);
        CHECK_FUNCTION_RETURN(VkCustomAPI::GetVulkanPhysicalDevice(vkInstance, vulkanSurface, vulkanRequiredDeviceExtensions, &vulkanPhysicalDevice, &vulkanSelectedPhysicalDeviceQueueFamily), true);

        LogInfo("Selected Physical Device Information: ");
        VkCustomAPI::LogPhysicalDeviceInfo(vulkanPhysicalDevice);
        // Setup Logical Device
        CHECK_FUNCTION_RETURN(VkCustomAPI::CreateVulkanLogicalDevice(vulkanPhysicalDevice, vulkanSelectedPhysicalDeviceQueueFamily, vulkanRequiredDeviceExtensions, &vulkanLogicalDevice), true);

        ///////////////// Retrieving Queue Handles
        vkGetDeviceQueue(vulkanLogicalDevice, vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily, 0, &queues.graphics_queue);
        vkGetDeviceQueue(vulkanLogicalDevice, vulkanSelectedPhysicalDeviceQueueFamily.presentFamily,  0, &queues.present_queue );
        vkGetDeviceQueue(vulkanLogicalDevice, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily, 0, &queues.transfer_queue);

        // Create Vulkan Command Pool
            // we will be recording a command buffer every frame, so we want to be able to reset and re-record over it.
            // We're going to record commands for drawing, which is why we've chosen the graphics queue family.
        CHECK_FUNCTION_RETURN(VkUtil::CreateVkCommandPool(vulkanLogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, vulkanSelectedPhysicalDeviceQueueFamily.graphicsFamily, &vulkanGraphicsCommandPool), true);

            // Apply memory allocation optimizations, for that use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
        CHECK_FUNCTION_RETURN(VkUtil::CreateVkCommandPool(vulkanLogicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily, &vulkanTransferCommandPool), true);

        // Create Command Buffers
        CHECK_FUNCTION_RETURN(VkUtil::CreateVkCommandBuffers(vulkanLogicalDevice, vulkanGraphicsCommandPool, MAX_FRAMES_IN_FLIGHT, vulkanCommandBuffers), true);

        LogSuccess("Vulkan: [Success] Vulkan Device And Queue Create Successfully.");

        return true;
    }

    /**
     * @brief Initialize()
     */
    bool Initialize(HWND hwnd)
    {
        // code
        // Initialize Vulkan
        CHECK_FUNCTION_RETURN(InitializeVulkan(hwnd), true);

        // Create Swap Chain
        CHECK_FUNCTION_RETURN(CreateVulkanSwapChain(), true);

        // Create Image Views of SwapChain images
        CHECK_FUNCTION_RETURN(CreateVulkanImageViewsForSwapchain(), true);

        // Create Depth Buffer
        CHECK_FUNCTION_RETURN(CreateDepthResource(), true);

        // Create Render Pass
        CHECK_FUNCTION_RETURN(CreateVulkanRenderPass(), true);

        // Create Swapchain Framebuffer
        CHECK_FUNCTION_RETURN(CreateVulkanFramebuffersForSwapchain(), true);

        // ====================================================================
            // Model Loading
        CHECK_FUNCTION_RETURN( LoadGLTFModel("../Assets/models/teapot.gltf"), true);
            // Create Vertex Buffer
        CHECK_FUNCTION_RETURN(CreateVulkanBuffer(sizeof(modelVertices[0]) * modelVertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanModelVertexBuffer, (void*) modelVertices.data()), true);
            // Create Index Buffer
        CHECK_FUNCTION_RETURN(CreateVulkanBuffer(sizeof(modelIndices[0]) * modelIndices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanModelIndexBuffer, (void*) modelIndices.data()), true);
        // ====================================================================
        CHECK_FUNCTION_RETURN( LoadKtxTexture("../Assets/textures/metalplate_nomips_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM), true);
        // ====================================================================
            // Create Uniform Buffer
        CHECK_FUNCTION_RETURN(CreateUniformBuffer(sizeof(UniformData), vulkanUniformBuffers, MAX_FRAMES_IN_FLIGHT), true);
        // ====================================================================
            // Create Descriptor Pool
        CHECK_FUNCTION_RETURN(CreateDescriptorPool(), true);
            // Create Descriptor Set Layout
        CHECK_FUNCTION_RETURN(CreateVulkanDescriptorSetLayout(), true);
            // Create Descriptor Sets
        CHECK_FUNCTION_RETURN(CreateDescriptorSets(), true);
        // ====================================================================
            // Create Grpahics Pipeline
        CHECK_FUNCTION_RETURN(CreateVulkanGraphicsPipelines(), true);


        // Create Sync Objects
        CHECK_FUNCTION_RETURN(CreateVulkanSyncObjects(), true);

        return true;
    }

    /**
     * @brief ResizeCallback()
     */
    void ResizeCallback(int width, int height)
    {
        // code
        swapchain.framebufferResized = true;  // Handling Resize Explicitly
    }

    /**
     * @brief CleanupSwapChain()
     */
    static void CleanupSwapChain()
    {
        // code
        for(VkFramebuffer& framebuffer : swapchain.framebuffers)
        {
            if(framebuffer)
            {
                vkDestroyFramebuffer(vulkanLogicalDevice, framebuffer, nullptr);
                framebuffer = nullptr;
            }
        }

        for(VkImageView& imageView : swapchain.imageViews)
        {
            if(imageView)
            {
                vkDestroyImageView(vulkanLogicalDevice, imageView, nullptr);
                imageView = nullptr;
            }
        }

        if(swapchain.handle)
        {
            vkDestroySwapchainKHR(vulkanLogicalDevice, swapchain.handle, nullptr);
            swapchain.handle = nullptr;
        }

        // Depth buffer
        if(swapchain.depth_buffer.image)
        {
            vkDestroyImage(vulkanLogicalDevice, swapchain.depth_buffer.image, nullptr);
            swapchain.depth_buffer.image = nullptr;
        }

        if(swapchain.depth_buffer.imageMemory)
        {
            vkFreeMemory(vulkanLogicalDevice, swapchain.depth_buffer.imageMemory, nullptr);
            swapchain.depth_buffer.imageMemory = nullptr;
        }

        if(swapchain.depth_buffer.imageView)
        {
            vkDestroyImageView(vulkanLogicalDevice, swapchain.depth_buffer.imageView, nullptr);
            swapchain.depth_buffer.imageView = nullptr;
        }
    }

    /**
     * @brief RecreateSwapChain()
     */
    void RecreateSwapChain()
    {
        // function declaration
        bool PollWindowEvent();

        // code
        int width = 0, height = 0;
        RECT rc;
        GetClientRect(ghwnd, &rc);
        while(rc.right == 0 || rc.bottom == 0)
        {
            GetClientRect(ghwnd, &rc);
            WaitMessage();
            PollWindowEvent();
        }

            // We shouldn't touch resources that may still be in use. Obviously, we'll have to recreate the swap chain itself.
            // The image views need to be recreate because they are based directly on the swap chain images.
            // Finally, the framebuffers directly depend on the swap chain images, and thus must be recreated as well.
        vkDeviceWaitIdle(vulkanLogicalDevice);

            // To make sure that the old versions of these objects are cleaned up before recreating them.
        CleanupSwapChain();

        CreateVulkanSwapChain();
        CreateVulkanImageViewsForSwapchain();
        CreateDepthResource();
        CreateVulkanFramebuffersForSwapchain();
    }

    /**
     * @brief RecordCommandBuffer()
     */
    static bool RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex /* current swap chain image we want to write to. */)
    {
        // code
            // We always begin recording a command buffer by calling 'vkBeginCommandBuffer()'.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;    // Optional
        beginInfo.pInheritanceInfo = nullptr;   // Optional

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkBeginCommandBuffer( commandBuffer, &beginInfo), false);

////////////// Starting a render pass
        VkClearValue clearValues[2]{};
        clearValues[0].color = {   /* VkClearColorValue */ { 0.0f, 0.0f, 0.0f, 1.0f}   /* float32[4] */ };
        clearValues[1].depthStencil = { 1.0f, 0};

        // Drawing starts by beginning the render pass with vkCmdBeginRenderPass().
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanRenderPass;
        renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex];   // framebuffer for the swapchain image we want to draw to.
            // Specify size of the render area.
        renderPassInfo.renderArea.offset = { 0, 0};
        renderPassInfo.renderArea.extent = swapchain.extent;

        renderPassInfo.clearValueCount = _ARRAYSIZE(clearValues);
        renderPassInfo.pClearValues = clearValues;

            // begin render pass.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE /* hown drawing commands within the render pass will be provided. */);

////////////// Drawing Commands

        VkRect2D scissor{};
        scissor.offset = { 0, 0};
        scissor.extent = swapchain.extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind Descriptor [Uniform and Texture]
        vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineLayout, 0, 1, &vulkanDescriptorSets[currentFrame], 0, nullptr);

        // Bind Vertex Buffer
        VkDeviceSize offsets[] = {0};
        

        VkViewport viewport{};
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.width = (swapchain.extent.width / 3.0f);
        viewport.height = (float)(swapchain.extent.height);

        // Left
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.phong);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanModelVertexBuffer.handle, offsets);
        vkCmdBindIndexBuffer(commandBuffer, vulkanModelIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, (uint32_t)modelIndices.size(), 1, 0, 0, 0);

        // Center
        viewport.x = (swapchain.extent.width / 3.0f);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.toon);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanModelVertexBuffer.handle, offsets);
        vkCmdBindIndexBuffer(commandBuffer, vulkanModelIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, (uint32_t)modelIndices.size(), 1, 0, 0, 0);

        // Right
        viewport.x = 2.0f * (swapchain.extent.width / 3.0f);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.textured);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanModelVertexBuffer.handle, offsets);
        vkCmdBindIndexBuffer(commandBuffer, vulkanModelIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, (uint32_t)modelIndices.size(), 1, 0, 0, 0);

////////////// Finishing Up
        // The render pass can now be ended.
        vkCmdEndRenderPass(commandBuffer);

        // finish recording the command buffer
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkEndCommandBuffer(commandBuffer), false);
        return true;
    }

    /**
     * @brief UpdateUniformBuffer()
     */
    static void UpdateUniformBuffer(uint32_t currentImage)
    {
        // code
        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        uniform_data.projection = glm::perspective(glm::radians(60.0f), ((float)swapchain.extent.width / 3.0f) / (float)swapchain.extent.height, 0.1f, 512.0f);
        uniform_data.view = glm::lookAt( glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        uniform_data.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            // GLM was originally designed for OpenGL, where teh U coordinate of the clip coordinates is inverted. The easiest way to
            // compendate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix.
        uniform_data.projection[1][1] *= -1;

        // copy data to uniform buffer
        memcpy(vulkanUniformBuffers[currentImage].mapped, &uniform_data, sizeof(uniform_data));
    }

    static uint32_t PrepareFrame()
    {
    // Waiting for the previous frame.
        vkWaitForFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE /* Wait for all fences */, UINT64_MAX /* Timeout */);

    // Acquiring and image from the swap chain.
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            vulkanLogicalDevice,
            swapchain.handle,
            UINT64_MAX /* timeout in nanoseconds for an image to become available (disable timeout here)*/,
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE,
            &imageIndex
        );

    // Check if swap chain has become incompatible with the surface and can no longer be used for rendering. (Usually happen for resize).
        if(result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();    // recreate swap chain and try again in next DrawFrame() call.
            return UINT32_MAX;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LogError("Failed to acquire swap chain image!!!");
            return UINT32_MAX;
        }

    // Manually reset the fence to the unsignaled state.
        vkResetFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame]);

        return imageIndex;
    }

    static void SubmitFrame(uint32_t imageIndex)
    {
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

    //  Submitting the command buffer
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vulkanCommandBuffers[currentFrame];  // Command Buffers to actually submit for execution.
        
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;    // Specify which semaphores to wait on before execution begins.
        submitInfo.pWaitDstStageMask = waitStages;      // In which stage(s) of the pipeline to wait.
            // Each entry in the 'waitStage' array corresponds to the semaphore with the same index in 'pWaitSemaphores'

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;    // Specify which semaphores to signal once the command buffer(s) have finished execution.

        VK_UTIL_FN_ERROR_CHECK_RETURN( vkQueueSubmit( queues.graphics_queue, 1, &submitInfo, inFlightFences[currentFrame]), );

// Presentation
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.handle;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(queues.present_queue, &presentInfo);

        // Check if swap chain has become incompatible with the surface and can no longer be used for rendering. (Usually happen for resize).
        if(swapchain.framebufferResized)
        {
            swapchain.framebufferResized = false;

            // recreate swap chain and try again in next DrawFrame() call.
            RecreateSwapChain();
            return;
        }
        else if(result != VK_SUCCESS)
        {
            LogError("Failed to present swap chain image!!!");
            return;
        }

        // we render current frame and it sends to GPU for process,
        // instead of waiting for GPU to finish, we start rendering next frame.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    /**
     * @brief DrawFrame()
     */
    void DrawFrame()
    {
        uint32_t imageIndex = PrepareFrame();
        if(imageIndex == UINT32_MAX)
        {
            return;
        }

    // Update Uniforms
        UpdateUniformBuffer(currentFrame);

    // Build Command Buffer
    // Recording the command buffer
        vkResetCommandBuffer( vulkanCommandBuffers[currentFrame], 0);  // Make sure it is able to be recorded.
        RecordCommandBuffer(vulkanCommandBuffers[currentFrame], imageIndex);

    // Submit Frame
        SubmitFrame(imageIndex);
    }

    /**
     * @brief Unintialize()
     */
    void Uninitialize()
    {
        // code
    // Wait for the logical device to finish operations before existing
        if(vulkanLogicalDevice)
        {
            vkDeviceWaitIdle(vulkanLogicalDevice);
        }

        CleanupSwapChain();

        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if(vulkanUniformBuffers[i].handle)
            {
                vkDestroyBuffer(vulkanLogicalDevice, vulkanUniformBuffers[i].handle, nullptr);
                vulkanUniformBuffers[i].handle = nullptr;
            }

            if(vulkanUniformBuffers[i].memory)
            {
                vkUnmapMemory(vulkanLogicalDevice, vulkanUniformBuffers[i].memory);
                vulkanUniformBuffers[i].mapped = nullptr;

                vkFreeMemory(vulkanLogicalDevice, vulkanUniformBuffers[i].memory, nullptr);
                vulkanUniformBuffers[i].memory = nullptr;
            }
        }

        if(vulkanDescriptorPool)
        {
            vkDestroyDescriptorPool(vulkanLogicalDevice, vulkanDescriptorPool, nullptr);
            vulkanDescriptorPool = nullptr;
        }

        if(vulkanDescriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout( vulkanLogicalDevice, vulkanDescriptorSetLayout, nullptr);
            vulkanDescriptorSetLayout = nullptr;
        }

        if(vulkanModelVertexBuffer.handle)
        {
            vkDestroyBuffer(vulkanLogicalDevice, vulkanModelVertexBuffer.handle, nullptr);
            vulkanModelVertexBuffer.handle = nullptr;
        }

        if(vulkanModelVertexBuffer.memory)
        {
            vkFreeMemory(vulkanLogicalDevice, vulkanModelVertexBuffer.memory, nullptr);
            vulkanModelVertexBuffer.memory = nullptr;
        }

        if(vulkanModelIndexBuffer.handle)
        {
            vkDestroyBuffer(vulkanLogicalDevice, vulkanModelIndexBuffer.handle, nullptr);
            vulkanModelIndexBuffer.handle = nullptr;
        }

        if(vulkanModelIndexBuffer.memory)
        {
            vkFreeMemory(vulkanLogicalDevice, vulkanModelIndexBuffer.memory, nullptr);
            vulkanModelIndexBuffer.memory = nullptr;
        }

        if(vulkanModelTexture)
        {
            vkDestroyImage(vulkanLogicalDevice, vulkanModelTexture, nullptr);
            vulkanModelTexture = nullptr;
        }

        if(vulkanModelTextureMemory)
        {
            vkFreeMemory(vulkanLogicalDevice, vulkanModelTextureMemory, nullptr);
            vulkanModelTextureMemory = nullptr;
        }

        if(vulkanModelTextureView)
        {
            vkDestroyImageView(vulkanLogicalDevice, vulkanModelTextureView, nullptr);
            vulkanModelTextureView = nullptr;
        }

        if(vulkanModelTextureSampler)
        {
            vkDestroySampler(vulkanLogicalDevice, vulkanModelTextureSampler, nullptr);
            vulkanModelTextureSampler = nullptr;
        }

        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if(imageAvailableSemaphores[i])
            {
                vkDestroySemaphore(vulkanLogicalDevice, imageAvailableSemaphores[i], nullptr);
                imageAvailableSemaphores[i] = nullptr;
            }

            if(renderFinishedSemaphores[i])
            {
                vkDestroySemaphore(vulkanLogicalDevice, renderFinishedSemaphores[i], nullptr);
                renderFinishedSemaphores[i] = nullptr;
            }

            if(inFlightFences[i])
            {
                vkDestroyFence(vulkanLogicalDevice, inFlightFences[i], nullptr);
                inFlightFences[i] = nullptr;
            }
        }

        if(vulkanCommandBuffers[0])
        {
            vkFreeCommandBuffers(vulkanLogicalDevice, vulkanGraphicsCommandPool, MAX_FRAMES_IN_FLIGHT, vulkanCommandBuffers);

            for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                vulkanCommandBuffers[i] = nullptr;
            }
        }
        
        if(vulkanGraphicsCommandPool)
        {
            vkDestroyCommandPool(vulkanLogicalDevice, vulkanGraphicsCommandPool, nullptr);
            vulkanGraphicsCommandPool = nullptr;
        }

        if(vulkanTransferCommandPool)
        {
            vkDestroyCommandPool(vulkanLogicalDevice, vulkanTransferCommandPool, nullptr);
            vulkanTransferCommandPool = nullptr;
        }

        if(pipelines.phong)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.phong, nullptr);
            pipelines.phong = nullptr;
        }

        if(pipelines.textured)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.textured, nullptr);
            pipelines.textured = nullptr;
        }

        if(pipelines.toon)
        {
            vkDestroyPipeline( vulkanLogicalDevice, pipelines.toon, nullptr);
            pipelines.toon = nullptr;
        }

        if(vulkanPipelineLayout)
        {
            vkDestroyPipelineLayout(vulkanLogicalDevice, vulkanPipelineLayout, nullptr);
            vulkanPipelineLayout = nullptr;
        }

        if(vulkanRenderPass)
        {
            vkDestroyRenderPass( vulkanLogicalDevice, vulkanRenderPass, nullptr);
            vulkanRenderPass = nullptr;
        }

        if(vulkanLogicalDevice)
        {
            vkDestroyDevice(vulkanLogicalDevice, nullptr); // Logical Devices don't interact directly with instances.
            vulkanLogicalDevice = nullptr;
        }

        if(vulkanSurface)
        {
            vkDestroySurfaceKHR(vkInstance, vulkanSurface, nullptr);
            vulkanSurface = nullptr;
        }

        VkUtil::DestroyVulkanInstance(&vkInstance);
    }
    
} // namespace VkApplication
