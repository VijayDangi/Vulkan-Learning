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
#include "vx_utils.h"
#include "vx_initializers.hpp"

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


namespace VkApplication
{
    struct Particle
    {
        glm::vec4 Position;
        glm::vec3 Velocity;
        float     Age;
        float     MaxAge;
        glm::vec2 Padding;    // Padding to make the struct size a multiple of
    };

    struct GraphicsPipelineUniformBufferObject
    {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjMatrix;
    };

    struct ComputePipelineUniformBufferObject
    {
        float Time;
        float DeltaTime;
        float MinAABB;
        float MaxAABB;
    };

    // Variable Declaration
    const std::vector<const char*> vulkanRequiredInstanceLayers = {
#if VK_ENABLE_VALIDATION_LAYER
        "VK_LAYER_KHRONOS_validation"
#endif
    };
    const std::vector<const char*> vulkanRequiredInstanceExtesion = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if VK_ENABLE_VALIDATION_LAYER
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    const std::vector<const char*> vulkanRequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    const int MAX_FRAMES_IN_FLIGHT = 2; // Defines how many frames should be processed concurrently.
    uint32_t currentFrame = 0;  // Track of the current frame.

    // Vulkan Specific Variables
    struct Version
    {
        int Major = 0;
        int Minor = 0;
        int Patch = 0;
        uint32_t PackedVersion;
    } vkInstanceVersion;

    struct {
        VkPhysicalDevice                 handle;
        VkPhysicalDeviceProperties       deviceProps;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkPhysicalDeviceFeatures         features;

        VkFormat                         depthFormat;
    } physicalDevice;

    VkInstance          vkInstance{ VK_NULL_HANDLE };
    VkSurfaceKHR        vulkanSurface{ VK_NULL_HANDLE };
    // VkPhysicalDevice    physicalDevice.handle{ VK_NULL_HANDLE }; // This object will be implicitly destroyedd when 'VkInstance' is destroyed.
    VkDevice            vulkanLogicalDevice{ VK_NULL_HANDLE };  // Vulkan Logical Device

    VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices vulkanSelectedPhysicalDeviceQueueFamily;

    VkQueue vulkanGraphicsComputeQueue{ VK_NULL_HANDLE };        // Devices queue are implicitly cleaned up when device is destroyed.
    VkQueue vulkanPresentationQueue{ VK_NULL_HANDLE };    // Devices queue are implicitly cleaned up when device is destroyed.

    struct {
        VkSwapchainKHR              handle;
        
        std::vector<VkImage>        images;
        std::vector<VkImageView>    imageViews;
        VkFormat                    imageFormat;
        VkExtent2D                  extent;

        VkPresentModeKHR            presentMode;
        std::vector<VkFramebuffer>  framebuffers;
        bool framebufferResized = false;
    } swapChain{};

    VX_FRAMEWORK_NAMESPACE::types::VulkanImage swapChainDepthTexture;
    VkImageView swapChainDepthView;

    struct {
        std::vector<VkFramebuffer>  framebuffers_MSAA;
        VX_FRAMEWORK_NAMESPACE::types::VulkanImage colorTexture;
        VkImageView colorView;
        VX_FRAMEWORK_NAMESPACE::types::VulkanImage depthTexture;
        VkImageView depthView;

        VkRenderPass renderPass_MSAA;
        bool useMSAA = false;
        bool requestedMSAA = false;
    } msaa;
    VkSampleCountFlagBits msaaSamplesCount = VK_SAMPLE_COUNT_1_BIT;

    struct {
        uint32_t framebufferWidth;
        uint32_t framebufferHeight;

        // VkCommandBuffer commandBuffer[MAX_FRAMES_IN_FLIGHT]{VK_NULL_HANDLE};
        VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    } imgui_renderer{};

    VkRenderPass vulkanRenderPass{ VK_NULL_HANDLE };

    VkCommandPool vulkanCommandPool{ VK_NULL_HANDLE };
    VkCommandBuffer vulkanOneTimeCommandBuffer{ VK_NULL_HANDLE };    // Command buffer for one time operations like texture loading, buffer copy etc. This command buffer will be automatically freed when its command pool is destroyed, so we don't need explicit clenup.
    VkCommandBuffer vulkanCommandBuffers[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // Command buffers will be automatically freed when their command pool is destroyed, so we don't need explicit clenup.


    VkDescriptorPool vulkanDescriptorPool{ VK_NULL_HANDLE };

        // Synchromization Objects
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // To signal that an image has been aquired from the swapchain and is ready for rendering.
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };    // To signal that rendering has finished and presentation can happen.
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };                  // To make sure only one frame is rendering at a time.

        // Particles Data
    const uint32_t MAX_PARTICLES = 1000000;
    const float MIN_AABB = -10.0f;
    const float MAX_AABB = 10.0f;

        // Shader Storage Buffer Object
    VX_FRAMEWORK_NAMESPACE::types::VulkanBuffer particleSSBOs[2];   // Ping Pong SSBOs for Compute Pipeline.

        // Graphics Pipeline
    struct GraphicsPipeline
    {
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        VkPipeline pipeline_MSAA{ VK_NULL_HANDLE };
        VkPipeline pipeline{ VK_NULL_HANDLE };
        VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet descriptorSets[2]{ VK_NULL_HANDLE };    // Ping Pong Descriptor Sets for Graphics Pipeline.
        VX_FRAMEWORK_NAMESPACE::types::VulkanUniformBuffer uniformBuffer{};
    } graphicsPipeline;

    struct ComputePipeline
    {
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        VkPipeline pipeline{ VK_NULL_HANDLE };
        VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet descriptorSets[2]{ VK_NULL_HANDLE };    // Ping Pong Descriptor Sets for Compute Pipeline.
        VX_FRAMEWORK_NAMESPACE::types::VulkanUniformBuffer uniformBuffer{};
    } computePipeline;

    // Sync Counter For Compute Pipeline
    VkSemaphore computePipelineFinishedSemaphore[MAX_FRAMES_IN_FLIGHT]{ VK_NULL_HANDLE };

    int computePingPongIndex = 0;
    int graphicsPingPongIndex = 0;
    uint32_t frameNumber = 0;

    bool reInitImGui = false;    // A flag to indicate whether we need to re-initialize Imgui. We need this when swapchain is recreated because Imgui's render resources are dependent on swapchain's render pass and framebuffers.
    float cameraDistance = 20.0f;

// ====================================================== Function Definition Begin ====================================================== //

    /**
     * @brief InitializeVulkan()
     */
    bool InitializeVulkan(HWND windowHandle)
    {
#define CHECK_FUNCTION_RETURN(x) \
        if(!(x))    \
        {   \
            LogError(#x " Failed.");    \
            return false;   \
        }

        // code
        uint32_t instanceVersion = 0;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateInstanceVersion(&instanceVersion), false);
        vkInstanceVersion.Major = VK_API_VERSION_MAJOR(instanceVersion);
        vkInstanceVersion.Minor = VK_API_VERSION_MINOR(instanceVersion);
        vkInstanceVersion.Patch = VK_API_VERSION_PATCH(instanceVersion);
        vkInstanceVersion.PackedVersion = instanceVersion;
        LogDebug("Vulkan Instance Version Supported: %d.%d.%d", vkInstanceVersion.Major, vkInstanceVersion.Minor, vkInstanceVersion.Patch);

        // Create Vulkan Instance
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVulkanInstance(vulkanRequiredInstanceLayers.data(), vulkanRequiredInstanceLayers.size(), vulkanRequiredInstanceExtesion.data(), (uint32_t)vulkanRequiredInstanceExtesion.size(), VK_ENABLE_VALIDATION_LAYER, &vkInstance));

        // Create Surface
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVulkanSurface(vkInstance, windowHandle, &vulkanSurface));

        // Get Physical Device
        VX_FRAMEWORK_NAMESPACE::helper::LogVulkanPhysicalDevicesInfo(vkInstance);
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::GetVulkanPhysicalDevice(vkInstance, vulkanSurface, vulkanRequiredDeviceExtensions, &physicalDevice.handle, &vulkanSelectedPhysicalDeviceQueueFamily));

        vkGetPhysicalDeviceProperties(physicalDevice.handle, &physicalDevice.deviceProps);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice.handle, &physicalDevice.memoryProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice.handle, &physicalDevice.features);

        // Get Depth Formats
        VkFormat depthFormatCandidates[] = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };

        physicalDevice.depthFormat = VX_FRAMEWORK_NAMESPACE::helper::FindSupportedFormat( physicalDevice.handle, depthFormatCandidates, _ARRAYSIZE(depthFormatCandidates), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        LogDebug("Depth Supported Format \"%s\"", VX_FRAMEWORK_NAMESPACE::helper::GetVkFormatString(physicalDevice.depthFormat));

        // Get MSAA Samples Count
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties( physicalDevice.handle, &physicalDeviceProperties);

        // get sample counts which are supported by both color and depth buffer.
        VkSampleCountFlags sampleCounts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
            
             if(sampleCounts & VK_SAMPLE_COUNT_64_BIT)      msaaSamplesCount = VK_SAMPLE_COUNT_64_BIT;
        else if(sampleCounts & VK_SAMPLE_COUNT_32_BIT)      msaaSamplesCount = VK_SAMPLE_COUNT_32_BIT;
        else if(sampleCounts & VK_SAMPLE_COUNT_16_BIT)      msaaSamplesCount = VK_SAMPLE_COUNT_16_BIT;
        else if(sampleCounts & VK_SAMPLE_COUNT_8_BIT)       msaaSamplesCount = VK_SAMPLE_COUNT_8_BIT;
        else if(sampleCounts & VK_SAMPLE_COUNT_4_BIT)       msaaSamplesCount = VK_SAMPLE_COUNT_4_BIT;
        else if(sampleCounts & VK_SAMPLE_COUNT_2_BIT)       msaaSamplesCount = VK_SAMPLE_COUNT_2_BIT;
        else msaaSamplesCount = VK_SAMPLE_COUNT_1_BIT;

        // Setup Logical Device
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.sampleRateShading = VK_TRUE;   // Enable Sample Rate Shading feature for better quality of MSAA. This is not a mandatory feature to enable MSAA, but it can improve the quality of MSAA by doing shading at sample rate instead of pixel rate.
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVulkanLogicalDevice(physicalDevice.handle, &deviceFeatures, vulkanSelectedPhysicalDeviceQueueFamily, vulkanRequiredDeviceExtensions, &vulkanLogicalDevice));

        ///////////////// Retrieving Queue Handles
        vkGetDeviceQueue(vulkanLogicalDevice, vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily, 0, &vulkanGraphicsComputeQueue);
        vkGetDeviceQueue(vulkanLogicalDevice, vulkanSelectedPhysicalDeviceQueueFamily.presentFamily, 0, &vulkanPresentationQueue);

        LogSuccess("Vulkan: [Success] Vulkan Device And Queue Create Successfully.");

        return true;

#undef CHECK_FUNCTION_RETURN
    }

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
     * @brief CreateVulkanSWaoChain()
     * @return 
     */
    static bool InitializeSwapChain()
    {
        // code
        VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails swapChainSupport = VX_FRAMEWORK_NAMESPACE::utils::GetSwapChainSupportDetails(physicalDevice.handle, vulkanSurface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        LogDebug("Vulkan Swap Chain Details:");
        LogDebug("\tSurface Format: %s, Color Space: %s", VX_FRAMEWORK_NAMESPACE::helper::GetVkFormatString(surfaceFormat.format), VX_FRAMEWORK_NAMESPACE::helper::GetVkColorSpaceString(surfaceFormat.colorSpace));
        LogDebug("\tPresent Mode: %s", VX_FRAMEWORK_NAMESPACE::helper::GetVkPresentModeString(presentMode));
        LogDebug("\tImage Extent: %d x %d", extent.width, extent.height);

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
        uint32_t queueFamilyIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily, vulkanSelectedPhysicalDeviceQueueFamily.presentFamily};

        if(vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily != vulkanSelectedPhysicalDeviceQueueFamily.presentFamily)
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

        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateSwapchainKHR( vulkanLogicalDevice, &swapChainCreateInfo, nullptr, &swapChain.handle), false);
        LogSuccess("Vulkan: [Success] Vulkan SwapChain Create Successfully.");

        // Retrieving the swap chain images.
        imageCount = 0;
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapChain.handle, &imageCount, nullptr);
        swapChain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, swapChain.handle, &imageCount, swapChain.images.data());

        LogSuccess("Vulkan: [Success] %u Images Retrieved from Swap Chain.", imageCount);

        // Store the format and extent we've chosen for the swap chain images.
        swapChain.imageFormat = surfaceFormat.format;
        swapChain.presentMode = presentMode;
        swapChain.extent = extent;

        // Create Swapchain Image Views.
        swapChain.imageViews.resize(imageCount);

        for(size_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
        {
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkImageViews(
                vulkanLogicalDevice,
                swapChain.images[imageIndex],
                VK_IMAGE_VIEW_TYPE_2D,
                swapChain.imageFormat,
                {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                VK_IMAGE_ASPECT_COLOR_BIT,0, 1, 0, 1,
                swapChain.imageViews[imageIndex]
            ))
            {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief CreateDepthResource()
     * @return 
     */
    static bool CreateDepthResource()
    {
        // code
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice.handle, vulkanSurface, &surfaceCaps);

        // Create Depth Image
        if(!VX_FRAMEWORK_NAMESPACE::utils::CreateImage(
            vulkanLogicalDevice, physicalDevice.handle,
            surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1U,
            physicalDevice.depthFormat,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            false,
            &swapChainDepthTexture.imageInfo, &swapChainDepthTexture.handle, &swapChainDepthTexture.memory
        ))
        {
            LogError("Depth Image Creation Failed.");
            return false;
        }

        vkResetCommandBuffer(vulkanOneTimeCommandBuffer, 0);
        vkBeginCommandBuffer(vulkanOneTimeCommandBuffer, &VX_FRAMEWORK_NAMESPACE::initializers::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

        // Transition Depth Image Layout to 'VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL' so that it can be used as depth attachment in framebuffer.
        if(!VX_FRAMEWORK_NAMESPACE::utils::ImageMemoryBarrier( vulkanOneTimeCommandBuffer, swapChainDepthTexture.handle, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1U, 0, 1U))
        {
            LogError("Depth Image Memory Barrier Failed.");
            vkEndCommandBuffer(vulkanOneTimeCommandBuffer);
            return false;
        }

        vkEndCommandBuffer(vulkanOneTimeCommandBuffer);

        // Submit the one time command buffer and wait for it to finish as we need to use the depth image in the upcoming render pass creation.
        VkSubmitInfo submitInfo = VX_FRAMEWORK_NAMESPACE::initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vulkanOneTimeCommandBuffer;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkQueueSubmit(vulkanGraphicsComputeQueue, 1, &submitInfo, VK_NULL_HANDLE), false);
        vkQueueWaitIdle(vulkanGraphicsComputeQueue);

        // Create Depth Image View
        VkComponentMapping componentMapping = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkImageViews( vulkanLogicalDevice, swapChainDepthTexture.handle, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, componentMapping, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1, swapChainDepthView))
        {
            LogError("Depth Image View Creation Failed.");
            return false;
        }

        return true;
    }

    /**
     * @brief CreateVulkanFramebuffersForSwapchain()
    */
    static bool CreateVulkanFramebuffersForSwapchain()
    {
        // code
        swapChain.framebuffers.resize( swapChain.imageViews.size());
        for(size_t i = 0; i < swapChain.imageViews.size(); ++i)
        {
            VkImageView attachments[] = {
                swapChain.imageViews[i],
                swapChainDepthView
            };

            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkFramebuffer(vulkanLogicalDevice, vulkanRenderPass, _ARRAYSIZE(attachments), attachments, swapChain.extent.width, swapChain.extent.height, 1, &swapChain.framebuffers[i]))
            {
                LogError("Vulkan: [Error] VX_FRAMEWORK_NAMESPACE::utils::CreateVkFramebuffer() Failed. ");
                return false;
            }
        }

        LogSuccess("Vulkan: [Success] Vulkan Swapchain Frambuffers Created.");
        return true;
    }

    /**
     * @brief CreateVulkanRenderPass()
     * @return 
     */
    static bool CreateVulkanRenderPass()
    {
        // code
////////////// Attachment Description.
        VkAttachmentDescription colorAttachmentDescrition{};
        colorAttachmentDescrition.format         = swapChain.imageFormat;
        colorAttachmentDescrition.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescrition.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescrition.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescrition.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescrition.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescrition.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;        // Specifies which layout the image will have before the render pass begins.
        colorAttachmentDescrition.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // Specifies the layout to automatically transition to when the render pass finishes.

        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format         = physicalDevice.depthFormat;
        depthAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;        // Specifies which layout the image will have before the render pass begins.
        depthAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

////////////// Subpasses and attachment references.
            // A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the
            // contents of framebuffers in previous passes, for example a sequence of post-processing effects that are applied one after another.
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;        // Specifies which attachment to reference by its index in the attachment descriptions array.
        colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifies which layout we would like the attachment to have during a subpass that uses this reference.

        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;        // Specifies which attachment to reference by its index in the attachment descriptions array.
        depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Specifies

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments    = &colorAttachmentReference;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

#if 1
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass    = 0;
        subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
#endif

////////////// Render Pass
        VkAttachmentDescription attachments[] = { colorAttachmentDescrition, depthAttachmentDescription };

        VkRenderPassCreateInfo renderPassCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::renderPassCreateInfo();
        renderPassCreateInfo.attachmentCount = _ARRAYSIZE(attachments);
        renderPassCreateInfo.pAttachments = attachments;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;

#if 1
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
#endif

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateRenderPass( vulkanLogicalDevice, &renderPassCreateInfo, nullptr, &vulkanRenderPass), false);
        LogSuccess("Vulkan: [Success] Vulkan Graphics Render Pass Created.");

        return true;
    }


    static bool InitializeMSAAResources(bool reInit = true)
    {
        // code
        msaa.colorTexture.Destroy(vulkanLogicalDevice);
        msaa.depthTexture.Destroy(vulkanLogicalDevice);
        for(VkFramebuffer& framebuffer : msaa.framebuffers_MSAA)
        {
            vkDestroyFramebuffer(vulkanLogicalDevice, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }
        if(msaa.renderPass_MSAA != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(vulkanLogicalDevice, msaa.renderPass_MSAA, nullptr);
            msaa.renderPass_MSAA = VK_NULL_HANDLE;
        }
        if(msaa.colorView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(vulkanLogicalDevice, msaa.colorView, nullptr);
            msaa.colorView = VK_NULL_HANDLE;
        }
        if(msaa.depthView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(vulkanLogicalDevice, msaa.depthView, nullptr);
            msaa.depthView = VK_NULL_HANDLE;
        }

        if(!reInit)
        {
            return true;
        }


        VkSurfaceCapabilitiesKHR surfaceCaps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice.handle, vulkanSurface, &surfaceCaps);

        // Create Color Image
        {
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateImage(
                vulkanLogicalDevice, physicalDevice.handle,
                surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1U,
                swapChain.imageFormat,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                msaaSamplesCount,
                false,
                &msaa.colorTexture.imageInfo, &msaa.colorTexture.handle, &msaa.colorTexture.memory
            ))
            {
                LogError("MSAA Color Image Creation Failed.");
                return false;
            }

            // Create Depth Image View
            VkComponentMapping componentMapping = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkImageViews( vulkanLogicalDevice, msaa.colorTexture.handle, VK_IMAGE_VIEW_TYPE_2D, swapChain.imageFormat, componentMapping, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, msaa.colorView))
            {
                LogError("MSAA Color Image View Creation Failed.");
                return false;
            }
        }

        // Create Depth Image
        {
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateImage(
                vulkanLogicalDevice, physicalDevice.handle,
                surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1U,
                physicalDevice.depthFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                msaaSamplesCount,
                false,
                &msaa.depthTexture.imageInfo, &msaa.depthTexture.handle, &msaa.depthTexture.memory
            ))
            {
                LogError("MSAA Depth Image Creation Failed.");
                return false;
            }

            // Create Depth Image View
            VkComponentMapping componentMapping = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkImageViews( vulkanLogicalDevice, msaa.depthTexture.handle, VK_IMAGE_VIEW_TYPE_2D, physicalDevice.depthFormat, componentMapping, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1, msaa.depthView))
            {
                LogError("MSAA Depth Image View Creation Failed.");
                return false;
            }
        }

        vkResetCommandBuffer(vulkanOneTimeCommandBuffer, 0);
        vkBeginCommandBuffer(vulkanOneTimeCommandBuffer, &VX_FRAMEWORK_NAMESPACE::initializers::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

        // Transition Depth Image Layout to 'VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL' so that it can be used as depth attachment in framebuffer.
        if(!VX_FRAMEWORK_NAMESPACE::utils::ImageMemoryBarrier( vulkanOneTimeCommandBuffer, msaa.colorTexture.handle, swapChain.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1U, 0, 1U))
        {
            LogError("Color Image Memory Barrier Failed.");
            vkEndCommandBuffer(vulkanOneTimeCommandBuffer);
            return false;
        }

        // Transition Depth Image Layout to 'VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL' so that it can be used as depth attachment in framebuffer.
        if(!VX_FRAMEWORK_NAMESPACE::utils::ImageMemoryBarrier( vulkanOneTimeCommandBuffer, msaa.depthTexture.handle, physicalDevice.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1U, 0, 1U))
        {
            LogError("Depth Image Memory Barrier Failed.");
            vkEndCommandBuffer(vulkanOneTimeCommandBuffer);
            return false;
        }

        vkEndCommandBuffer(vulkanOneTimeCommandBuffer);

        // Submit the one time command buffer and wait for it to finish as we need to use the depth image in the upcoming render pass creation.
        VkSubmitInfo submitInfo = VX_FRAMEWORK_NAMESPACE::initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vulkanOneTimeCommandBuffer;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkQueueSubmit(vulkanGraphicsComputeQueue, 1, &submitInfo, VK_NULL_HANDLE), false);
        vkQueueWaitIdle(vulkanGraphicsComputeQueue);


        // Create Render Pass
        {
            ////////////// Attachment Description.
            VkAttachmentDescription colorAttachmentDescription{};
            colorAttachmentDescription.format         = swapChain.imageFormat;
            colorAttachmentDescription.samples        = msaaSamplesCount;
            colorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depthAttachmentDescription{};
            depthAttachmentDescription.format         = physicalDevice.depthFormat;
            depthAttachmentDescription.samples        = msaaSamplesCount;
            depthAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription colorAttachmentResolverDescription{};
            colorAttachmentResolverDescription.format         = swapChain.imageFormat;
            colorAttachmentResolverDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentResolverDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolverDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentResolverDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolverDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentResolverDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentResolverDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            ////////////// Subpasses and attachment references.
            VkAttachmentReference colorAttachmentReference{};
            colorAttachmentReference.attachment = 0;
            colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentReference{};
            depthAttachmentReference.attachment = 1;
            depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentResolverReference{};
            colorAttachmentResolverReference.attachment = 2;
            colorAttachmentResolverReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDescription{};
            subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments    = &colorAttachmentReference;
            subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
            subpassDescription.pResolveAttachments = &colorAttachmentResolverReference;

    #if 1
            VkSubpassDependency subpassDependency{};
            subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
            subpassDependency.dstSubpass    = 0;
            subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    #endif

            ////////////// Render Pass
            VkAttachmentDescription attachments[] = { colorAttachmentDescription, depthAttachmentDescription, colorAttachmentResolverDescription };

            VkRenderPassCreateInfo renderPassCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::renderPassCreateInfo();
            renderPassCreateInfo.attachmentCount = _ARRAYSIZE(attachments);
            renderPassCreateInfo.pAttachments = attachments;
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpassDescription;

    #if 1
            renderPassCreateInfo.dependencyCount = 1;
            renderPassCreateInfo.pDependencies = &subpassDependency;
    #endif

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateRenderPass( vulkanLogicalDevice, &renderPassCreateInfo, nullptr, &msaa.renderPass_MSAA), false);
            LogSuccess("Vulkan: [Success] Vulkan Graphics Render Pass Created.");
        }

        // Framebuffers
        {
            msaa.framebuffers_MSAA.resize( swapChain.imageViews.size());
            for(size_t i = 0; i < msaa.framebuffers_MSAA.size(); ++i)
            {
                VkImageView attachments[] = { msaa.colorView, msaa.depthView, swapChain.imageViews[i]
                };

                if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkFramebuffer(vulkanLogicalDevice, msaa.renderPass_MSAA, _ARRAYSIZE(attachments), attachments, swapChain.extent.width, swapChain.extent.height, 1, &msaa.framebuffers_MSAA[i]))
                {
                    LogError("Vulkan: [Error] VX_FRAMEWORK_NAMESPACE::utils::CreateVkFramebuffer() Failed. ");
                    return false;
                }
            }

            LogSuccess("Vulkan: [Success] Vulkan Swapchain Frambuffers Created.");
        }

        return true;
    }

// ======================== IMGUI Integration : Begin ======================== //
    /**
     * @brief __check_imgui_vk_result_callback_()
     */
    void __check_imgui_vk_result_callback_(VkResult result)
    {
        if(result != VK_SUCCESS)
        {
            LogError("[ImGui Error]: (0x%X) %s", result, VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(result));
        }
    }

    bool InitializeImGui(HWND windowHandle)
    {
        // code
            // Crate Descriptor Pool for IMGUI
        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo poolCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorPoolCreateInfo( static_cast<uint32_t>(std::size(poolSizes)), poolSizes, 1000 * static_cast<uint32_t>(std::size(poolSizes)), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        VkResult result = vkCreateDescriptorPool(vulkanLogicalDevice, &poolCreateInfo, nullptr, &imgui_renderer.descriptorPool);
        if(result != VK_SUCCESS)
        {
            LogError("Failed to create ImGui Descriptor Pool. Error: (0x%X) %s", result, VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(result));
            return false;
        }

            // Initialize ImGui for Win32 and Vulkan
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(windowHandle);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;    // Enable setting the mouse position (used when ImGuiConfigFlags_NavEnableGamepad is enabled and you use a gamepad to navigate the UI, which will move the mouse cursor).
        io.DisplaySize.x = static_cast<float>(swapChain.extent.width);
        io.DisplaySize.y = static_cast<float>(swapChain.extent.height);

            // Setup ImGui Style
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg]          = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
        style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header]           = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_CheckMark]        = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_SliderGrab]       = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
        style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
        style.Colors[ImGuiCol_Button]           = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_ButtonActive]     = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

        // Initialize ImGui Vulkan Backend
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.handle, vulkanSurface, &surfaceCapabilities);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.ApiVersion = vkInstanceVersion.PackedVersion;
        initInfo.Instance = vkInstance;
        initInfo.PhysicalDevice = physicalDevice.handle;
        initInfo.Device = vulkanLogicalDevice;
        initInfo.QueueFamily = vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily;
        initInfo.Queue = vulkanGraphicsComputeQueue;
        initInfo.DescriptorPool = imgui_renderer.descriptorPool;
        initInfo.MinImageCount = surfaceCapabilities.minImageCount;
        initInfo.ImageCount = static_cast<uint32_t>(swapChain.images.size());
        initInfo.PipelineCache = VK_NULL_HANDLE;

        initInfo.UseDynamicRendering = VK_FALSE;   // We are using Render Passes, so this should be false.
        
        initInfo.PipelineInfoMain.RenderPass    = msaa.useMSAA ? msaa.renderPass_MSAA : vulkanRenderPass;
        initInfo.PipelineInfoMain.Subpass       = 0;
        initInfo.PipelineInfoMain.MSAASamples   =  msaa.useMSAA ? msaaSamplesCount : VK_SAMPLE_COUNT_1_BIT;

        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pNext = nullptr;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.viewMask = 0;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChain.imageFormat;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = __check_imgui_vk_result_callback_;

        if(!ImGui_ImplVulkan_Init(&initInfo))
        {
            LogError("Failed to initialize ImGui Vulkan Backend.");
            return false;
        }

        // Command Buffer for uploading ImGui Fonts
        // if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkCommandBuffers(vulkanLogicalDevice, vulkanCommandPool, MAX_FRAMES_IN_FLIGHT, imgui_renderer.commandBuffer))
        // {
        //     LogError("Failed to create command buffer for ImGui font uploading.");
        //     return false;
        // }

        return true;
    }

    void ImGuiResize(uint32_t width, uint32_t height)
    {
        if(ImGui::GetCurrentContext())
        {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize.x = static_cast<float>(width);
            io.DisplaySize.y = static_cast<float>(height);
        }
    }

    void ImGuiDestroy()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // if(imgui_renderer.commandBuffer[0] != VK_NULL_HANDLE)
        // {
        //     vkFreeCommandBuffers(vulkanLogicalDevice, vulkanCommandPool, MAX_FRAMES_IN_FLIGHT, imgui_renderer.commandBuffer);
        //     memset(imgui_renderer.commandBuffer, 0, sizeof(imgui_renderer.commandBuffer));
        // }

        if(imgui_renderer.descriptorPool)
        {
            vkDestroyDescriptorPool(vulkanLogicalDevice, imgui_renderer.descriptorPool, nullptr);
            imgui_renderer.descriptorPool = VK_NULL_HANDLE;
        }
    }
// ======================== IMGUI Integration : End ======================== //


    /**
     * @brief CreateDescriptorPool()
     */
    static bool CreateDescriptorPool()
    {
        // code
        VkDescriptorPoolSize poolSize[] = {
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2)),
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2))
        };
        VkDescriptorPoolCreateInfo poolInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorPoolCreateInfo(static_cast<uint32_t>(std::size(poolSize)), poolSize, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2));

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDescriptorPool(vulkanLogicalDevice, &poolInfo, nullptr, &vulkanDescriptorPool), false);
        return true;
    }

    /**
     * @brief CreateVulkanSyncObjects()
     */
    static bool CreateVulkanSyncObjects()
    {
        // code
        VkSemaphoreCreateInfo semaphoreCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::semaphoreCreateInfo();
        VkFenceCreateInfo fenceCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);   // We create the fence in a signaled state, so that the first call to 'vkWaitForFences()' will not block.

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
    static bool CreateVulkanBuffer( VkDeviceSize maxBufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VX_FRAMEWORK_NAMESPACE::types::VulkanBuffer& buffer, void *data = nullptr, size_t dataSizeBytes = 0)
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

        uint32_t queueFamiliesIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily};
        uint32_t numQueues = 2;

        // Create required buffer
        if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkBuffer( physicalDevice.handle, vulkanLogicalDevice, maxBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, memoryProperties, queueFamiliesIndices, numQueues, buffer.handle, buffer.memory))
        {
            LogError("CreateBuffer() Failed.");
            return false;
        }

        // if there is data then copy it to buffer.
        if(data != nullptr)
        {
            VX_FRAMEWORK_NAMESPACE::types::VulkanBuffer stagingBuffer{};
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkBuffer( physicalDevice.handle, vulkanLogicalDevice, maxBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queueFamiliesIndices, numQueues, stagingBuffer.handle, stagingBuffer.memory))
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
            VX_FRAMEWORK_NAMESPACE::utils::CopyVkBuffer(vulkanLogicalDevice, vulkanCommandPool, vulkanGraphicsComputeQueue, stagingBuffer.handle, buffer.handle, dataSizeBytes);

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
    static bool CreateUniformBuffer( VkDeviceSize bufferSize, VX_FRAMEWORK_NAMESPACE::types::VulkanUniformBuffer *uniformBuffers, size_t maxFramesInFlight = MAX_FRAMES_IN_FLIGHT)
    {
        // code
        uint32_t queueFamiliesIndices[] = { vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily, vulkanSelectedPhysicalDeviceQueueFamily.transferFamily};
        uint32_t numQueues = 2;

        for(size_t i = 0; i < maxFramesInFlight; ++i)
        {
            if(!VX_FRAMEWORK_NAMESPACE::utils::CreateVkBuffer( physicalDevice.handle, vulkanLogicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, queueFamiliesIndices, numQueues, uniformBuffers[i].handle, uniformBuffers[i].memory))
            {
                LogError("[Error] CreateBuffer() Failed for Uniform at %d.", (int)i);
                return false;
            }

            vkMapMemory(vulkanLogicalDevice, uniformBuffers[i].memory, 0, bufferSize, 0, &uniformBuffers[i].mapped);
        }

        return true;
    }


    // ============== Graphics Pipeline Initialization : Begin ==============
    /**
     * @brief CreateGraphicsPipelineResources()
    */
    static bool CreateGraphicsPipelineResources()
    {
        // code
        // Create Uniform Buffers
        if(!CreateUniformBuffer(sizeof(GraphicsPipelineUniformBufferObject), &graphicsPipeline.uniformBuffer, 1U))
        {
            LogError("Failed to create uniform buffers for graphics pipeline.");
            return false;
        }

        // Descriptor Set Layouts
        VkDescriptorSetLayoutBinding descLayoutBindings[] = {
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutBinding(0U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1U),
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutBinding(1U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1U)
        };
        VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutCreateInfo( static_cast<uint32_t>(std::size(descLayoutBindings)), descLayoutBindings);
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDescriptorSetLayout( vulkanLogicalDevice, &descSetLayoutCreateInfo, nullptr, &graphicsPipeline.descriptorSetLayout), false);

        // Create Descriptor Sets
        VkDescriptorSetLayout layouts[2];
        for( int i = 0; i < 2; ++i)
        {
            layouts[i] = graphicsPipeline.descriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo allocInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetAllocateInfo(vulkanDescriptorPool, 2U, layouts);
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkAllocateDescriptorSets(vulkanLogicalDevice, &allocInfo, graphicsPipeline.descriptorSets), false);

        for( size_t i = 0; i < 2; ++i)
        {
            VkDescriptorBufferInfo storageBufferInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorBufferInfo(particleSSBOs[i].handle, 0, VK_WHOLE_SIZE);
            VkDescriptorBufferInfo uniformBufferInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorBufferInfo(graphicsPipeline.uniformBuffer.handle, 0, sizeof(GraphicsPipelineUniformBufferObject));

            VkWriteDescriptorSet descriptorWrites[] = {
                VX_FRAMEWORK_NAMESPACE::initializers::writeBufferDescriptorSet(graphicsPipeline.descriptorSets[i], 0U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &storageBufferInfo, 1U),
                VX_FRAMEWORK_NAMESPACE::initializers::writeBufferDescriptorSet(graphicsPipeline.descriptorSets[i], 1U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniformBufferInfo, 1U)
            };

            vkUpdateDescriptorSets(vulkanLogicalDevice, static_cast<uint32_t>(std::size(descriptorWrites)), descriptorWrites, 0, nullptr);
        }

////////////// Creating shader modules.
        VkShaderModule vertexShaderModule = VX_FRAMEWORK_NAMESPACE::utils::CreateVkShaderModuleFromFile(vulkanLogicalDevice, "shaders/render.vert.spv");
        VkShaderModule fragmentShaderModule = VX_FRAMEWORK_NAMESPACE::utils::CreateVkShaderModuleFromFile(vulkanLogicalDevice, "shaders/render.frag.spv");

        if(!vertexShaderModule || !fragmentShaderModule)
        {
            if(fragmentShaderModule)
            {
                vkDestroyShaderModule( vulkanLogicalDevice, fragmentShaderModule, nullptr);
                fragmentShaderModule = nullptr;
            }
            
            if(vertexShaderModule)
            {
                vkDestroyShaderModule( vulkanLogicalDevice, vertexShaderModule, nullptr);
                vertexShaderModule = nullptr;
            }

            return false;
        }

////////////// Shader Stage Creation. (Programmable Stage)
        VkPipelineShaderStageCreateInfo vertexShaderStageInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderModule, "main");
        VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderModule, "main");

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

////////////// Vertex Input
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineVertexInputStateCreateInfo(0, nullptr, 0, nullptr);

////////////// Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0U, VK_FALSE);

////////////// Dynamic State
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineDynamicStateCreateInfo(static_cast<uint32_t>(std::size(dynamicStates)), dynamicStates);

////////////// Viewports and scissors
        // VkViewport viewport{};
        // viewport.x = 0.0f;
        // viewport.y = 0.0f;
        // viewport.width = (float)swapChain.extent.width;
        // viewport.height = (float)swapChain.extent.height;
        // viewport.minDepth = 0.0f;
        // viewport.maxDepth = 1.0f;

        // VkRect2D scissor{};
        // scissor.offset = { 0, 0};
        // scissor.extent = swapChain.extent;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineViewportStateCreateInfo(1U, nullptr, 1U, nullptr);


////////////// Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        rasterizerStateCreateInfo.lineWidth                 = 1.0f;
        rasterizerStateCreateInfo.depthClampEnable          = VK_FALSE;
        rasterizerStateCreateInfo.rasterizerDiscardEnable   = VK_FALSE;     // if it true then geometry never passes through the rasterizer stage. (Disable any output to the framebuffer).
        rasterizerStateCreateInfo.depthBiasEnable           = VK_FALSE;
        rasterizerStateCreateInfo.depthBiasConstantFactor   = 0.0f;         // Optional
        rasterizerStateCreateInfo.depthBiasClamp            = 0.0f;         // Optional
        rasterizerStateCreateInfo.depthBiasSlopeFactor      = 0.0f;         // Optional

///////////// Depth Stencil State
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
        depthStencilStateCreateInfo.minDepthBounds = 0.0f; // Optional
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f; // Optional
        
////////////// Multisampling
#if 1
        // Enabling it requires enabling a GPU feature.
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.minSampleShading = 1.0f;          // Optional
        multisampleCreateInfo.pSampleMask = nullptr;            // Optional
        multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;      // Optional

         // Enabling it requires enabling a GPU feature.
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo_MSAAEnabled = VX_FRAMEWORK_NAMESPACE::initializers::pipelineMultisampleStateCreateInfo(msaaSamplesCount);
        multisampleCreateInfo_MSAAEnabled.sampleShadingEnable = VK_TRUE;
        multisampleCreateInfo_MSAAEnabled.minSampleShading = 0.2f;
        multisampleCreateInfo_MSAAEnabled.pSampleMask = nullptr;
        multisampleCreateInfo_MSAAEnabled.alphaToCoverageEnable = VK_FALSE;
        multisampleCreateInfo_MSAAEnabled.alphaToOneEnable = VK_FALSE;
#endif

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
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = VX_FRAMEWORK_NAMESPACE::initializers::pipelineColorBlendAttachmentState(VK_FALSE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);    // Contains the configuration per attached framebuffer.
        colorBlendAttachmentState.srcColorBlendFactor   = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstColorBlendFactor   = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.colorBlendOp          = VK_BLEND_OP_ADD;        // Optional
        colorBlendAttachmentState.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.alphaBlendOp          = VK_BLEND_OP_ADD;        // Optional


        // The VkPipelineColorBlendStateCreateInfo() references the array of structures for all of the framebuffers and
        // allows you to blend constants that you can use as blend factors in the aforementioned calculations.
        //
        // if you want to use the second method of blending (bitwise combination), then you should set 'logicOpEnable'
        // to VK_TRUE.
        // The bitwise operation can then be specified in the 'logicOp' field. Note that this will automatically disable
        // the first method, as if you had set 'blendEnable' to VK_FALSE for every attached framebuffer!
        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineColorBlendStateCreateInfo(1U, &colorBlendAttachmentState);
        colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlendingCreateInfo.blendConstants[0] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[1] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[2] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[3] = 0.0f;   // Optional

////////////// Pipeline Layout
        // The uniform values need to be specified during pipeline creation by creating VkPipelineLayout object.
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineLayoutCreateInfo(1U, &graphicsPipeline.descriptorSetLayout, 0U, nullptr);

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreatePipelineLayout(vulkanLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &graphicsPipeline.pipelineLayout), false);

////////////// Create Graphics Pipeline
        {
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::graphicsPipelineCreateInfo(graphicsPipeline.pipelineLayout, vulkanRenderPass, 0U);     // Finally renference to the render pass and the index of the sub pass where this graphics pipeline will be used.
            graphicsPipelineCreateInfo.stageCount           = 2;
            graphicsPipelineCreateInfo.pStages              = shaderStages;
            graphicsPipelineCreateInfo.pVertexInputState    = &vertexInputCreateInfo;
            graphicsPipelineCreateInfo.pInputAssemblyState  = &inputAssemblyCreateInfo;
            graphicsPipelineCreateInfo.pViewportState       = &viewportStateCreateInfo;
            graphicsPipelineCreateInfo.pRasterizationState  = &rasterizerStateCreateInfo;
            graphicsPipelineCreateInfo.pMultisampleState    = &multisampleCreateInfo;
            graphicsPipelineCreateInfo.pDepthStencilState   = &depthStencilStateCreateInfo;
            graphicsPipelineCreateInfo.pColorBlendState     = &colorBlendingCreateInfo;
            graphicsPipelineCreateInfo.pDynamicState        = &dynamicStateCreateInfo;

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline.pipeline), false);
        }

        {
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::graphicsPipelineCreateInfo(graphicsPipeline.pipelineLayout, msaa.renderPass_MSAA, 0U);     // Finally renference to the render pass and the index of the sub pass where this graphics pipeline will be used.
            graphicsPipelineCreateInfo.stageCount           = 2;
            graphicsPipelineCreateInfo.pStages              = shaderStages;
            graphicsPipelineCreateInfo.pVertexInputState    = &vertexInputCreateInfo;
            graphicsPipelineCreateInfo.pInputAssemblyState  = &inputAssemblyCreateInfo;
            graphicsPipelineCreateInfo.pViewportState       = &viewportStateCreateInfo;
            graphicsPipelineCreateInfo.pRasterizationState  = &rasterizerStateCreateInfo;
            graphicsPipelineCreateInfo.pMultisampleState    = &multisampleCreateInfo_MSAAEnabled;
            graphicsPipelineCreateInfo.pDepthStencilState   = &depthStencilStateCreateInfo;
            graphicsPipelineCreateInfo.pColorBlendState     = &colorBlendingCreateInfo;
            graphicsPipelineCreateInfo.pDynamicState        = &dynamicStateCreateInfo;

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline.pipeline_MSAA), false);
        }

        // Destroy Shader Module
        vkDestroyShaderModule( vulkanLogicalDevice, fragmentShaderModule, nullptr);
        vkDestroyShaderModule( vulkanLogicalDevice, vertexShaderModule, nullptr);

        LogSuccess("Vulkan: [Success] Vulkan Graphics Pipeline Created.");

        return true;
    }
    // ============== Graphics Pipeline Initialization : End ==============

    // ============== Compute Pipeline Initialization : Begin ==============
    /**
     * @brief CreateComputePipelineResources()
    */
    static bool CreateComputePipelineResources()
    {
        // code
        // Create Uniform Buffers
        if(!CreateUniformBuffer(sizeof(ComputePipelineUniformBufferObject), &computePipeline.uniformBuffer, 1U))
        {
            LogError("Failed to create uniform buffers for compute pipeline.");
            return false;
        }

        // Descriptor Set Layouts
        VkDescriptorSetLayoutBinding descLayoutBindings[] = {
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutBinding(0U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1U),
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutBinding(1U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1U),
            VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutBinding(2U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1U)
        };
        VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetLayoutCreateInfo( static_cast<uint32_t>(std::size(descLayoutBindings)), descLayoutBindings);
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDescriptorSetLayout( vulkanLogicalDevice, &descSetLayoutCreateInfo, nullptr, &computePipeline.descriptorSetLayout), false);

        // Create Descriptor Sets
        VkDescriptorSetLayout layouts[2];
        for( int i = 0; i < 2; ++i)
        {
            layouts[i] = computePipeline.descriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo allocInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorSetAllocateInfo(vulkanDescriptorPool, 2U, layouts);
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkAllocateDescriptorSets(vulkanLogicalDevice, &allocInfo, computePipeline.descriptorSets), false);

        for( size_t i = 0; i < 2; ++i)
        {
            VkDescriptorBufferInfo readStorageBufferInfo  = VX_FRAMEWORK_NAMESPACE::initializers::descriptorBufferInfo(particleSSBOs[ i         ].handle, 0, VK_WHOLE_SIZE);
            VkDescriptorBufferInfo writeStorageBufferInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorBufferInfo(particleSSBOs[(i + 1) % 2].handle, 0, VK_WHOLE_SIZE);
            VkDescriptorBufferInfo uniformBufferInfo = VX_FRAMEWORK_NAMESPACE::initializers::descriptorBufferInfo(computePipeline.uniformBuffer.handle, 0, sizeof(ComputePipelineUniformBufferObject));

            VkWriteDescriptorSet descriptorWrites[] = {
                VX_FRAMEWORK_NAMESPACE::initializers::writeBufferDescriptorSet(computePipeline.descriptorSets[i], 0U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &readStorageBufferInfo, 1U),
                VX_FRAMEWORK_NAMESPACE::initializers::writeBufferDescriptorSet(computePipeline.descriptorSets[i], 1U, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &writeStorageBufferInfo, 1U),
                VX_FRAMEWORK_NAMESPACE::initializers::writeBufferDescriptorSet(computePipeline.descriptorSets[i], 2U, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniformBufferInfo, 1U)
            };

            vkUpdateDescriptorSets(vulkanLogicalDevice, static_cast<uint32_t>(std::size(descriptorWrites)), descriptorWrites, 0, nullptr);
        }

        // Create Compute Sync Objects
        VkSemaphoreCreateInfo semaphoreCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::semaphoreCreateInfo();

        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateSemaphore(vulkanLogicalDevice, &semaphoreCreateInfo, nullptr, &computePipelineFinishedSemaphore[i]), false);
        }

        // Create Compute Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineLayoutCreateInfo(1U, &computePipeline.descriptorSetLayout, 0U, nullptr);
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreatePipelineLayout(vulkanLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &computePipeline.pipelineLayout), false);

        // Create Compute Pipeline
        VkShaderModule computeShaderModule = VX_FRAMEWORK_NAMESPACE::utils::CreateVkShaderModuleFromFile(vulkanLogicalDevice, "shaders/advect.comp.spv");
        if(!computeShaderModule)
        {
            return false;
        }

        VkPipelineShaderStageCreateInfo computeShaderStageInfo = VX_FRAMEWORK_NAMESPACE::initializers::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeShaderModule, "main");
        VkComputePipelineCreateInfo computePipelineCreateInfo = VX_FRAMEWORK_NAMESPACE::initializers::computePipelineCreateInfo(computePipeline.pipelineLayout);
        computePipelineCreateInfo.stage = computeShaderStageInfo;
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateComputePipelines(vulkanLogicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &computePipeline.pipeline), false);

        // Destroy Shader Module
        vkDestroyShaderModule( vulkanLogicalDevice, computeShaderModule, nullptr);
        LogSuccess("Vulkan: [Success] Vulkan Compute Pipeline Created.");

        return true;
    }
    // ============== Compute Pipeline Initialization : End ==============

    float GetRandomValue()
    {
        //code
        return float(rand()) / float(RAND_MAX);
    }

    // GetRandomValueInRange()
    float GetRandomValueInRange(float min, float max)
    {
        //code
        return min + GetRandomValue() * (max - min);
    }

    /**
     * @brief Initialize()
     */
    bool Initialize(HWND hwnd)
    {
#define CHECK_FUNCTION_RETURN(x) \
        if(!(x))    \
        {   \
            LogError(#x " Failed.");    \
            return false;   \
        }

        // code
        // Initialize Vulkan
        CHECK_FUNCTION_RETURN(InitializeVulkan(hwnd));

        // Create Vulkan Command Pool
            // we will be recording a command buffer every frame, so we want to be able to reset and re-record over it.
            // We're going to record commands for drawing, which is why we've chosen the graphics queue family (also supports transfer operations, so we can use it for the transfer command pool as well).
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVkCommandPool(vulkanLogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily, &vulkanCommandPool));

        // Create One Time Command Buffers for setup operations (like copying buffers, etc.)
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVkCommandBuffers(vulkanLogicalDevice, vulkanCommandPool, 1, &vulkanOneTimeCommandBuffer));

        // Create Descriptor Pool
        CHECK_FUNCTION_RETURN(CreateDescriptorPool());

        // Create Swap Chain
        CHECK_FUNCTION_RETURN(InitializeSwapChain());

        // Create Depth Render Chain
        CHECK_FUNCTION_RETURN(CreateDepthResource());

        // Create Render Pass
        CHECK_FUNCTION_RETURN(CreateVulkanRenderPass());

        // Create Swapchain Framebuffer
        CHECK_FUNCTION_RETURN(CreateVulkanFramebuffersForSwapchain());

        // MSAA Resources
        CHECK_FUNCTION_RETURN(InitializeMSAAResources());

        // Initialize ImGui
        CHECK_FUNCTION_RETURN(InitializeImGui(hwnd));

        // Create Command Buffers
        CHECK_FUNCTION_RETURN(VX_FRAMEWORK_NAMESPACE::utils::CreateVkCommandBuffers(vulkanLogicalDevice, vulkanCommandPool, MAX_FRAMES_IN_FLIGHT, vulkanCommandBuffers));

        // Create Sync Objects
        CHECK_FUNCTION_RETURN(CreateVulkanSyncObjects());


        // Initialize Scene
        std::vector<Particle> particles(MAX_PARTICLES);
        for(Particle& particle : particles)
        {
            particle.Position = glm::vec4(GetRandomValueInRange(MIN_AABB, MAX_AABB), GetRandomValueInRange(MIN_AABB, MAX_AABB), GetRandomValueInRange(MIN_AABB, MAX_AABB), 1.0f);
            particle.Velocity = glm::vec4(0.0f);
            particle.Age = 0.0f;
            particle.MaxAge = GetRandomValueInRange(5.0f, 10.0f);
        }

            // Create SSBOs
        CHECK_FUNCTION_RETURN( CreateVulkanBuffer( sizeof(Particle) * MAX_PARTICLES, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particleSSBOs[0], particles.data(), sizeof(Particle) * MAX_PARTICLES));
        CHECK_FUNCTION_RETURN( CreateVulkanBuffer( sizeof(Particle) * MAX_PARTICLES, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, particleSSBOs[1], nullptr, 0U));

            // Create Graphics Pipeline
        CHECK_FUNCTION_RETURN(CreateGraphicsPipelineResources());

            // Create Compute Pipeline
        CHECK_FUNCTION_RETURN(CreateComputePipelineResources());

        return true;

#undef CHECK_FUNCTION_RETURN
    }

    /**
     * @brief ResizeCallback()
     */
    void ResizeCallback(int width, int height)
    {
        // code
        swapChain.framebufferResized = true;  // Handling Resize Explicitly

        ImGuiResize(width, height);
    }

    /**
     * @brief CleanupSwapChain()
     */
    static void CleanupSwapChain()
    {
        // code
            // Framebuffers
        for(VkFramebuffer& framebuffer : swapChain.framebuffers)
        {
            if(framebuffer)
            {
                vkDestroyFramebuffer(vulkanLogicalDevice, framebuffer, nullptr);
                framebuffer = nullptr;
            }
        }

            // Depth Resources
        swapChainDepthTexture.Destroy(vulkanLogicalDevice);
        if(swapChainDepthView)
        {
            vkDestroyImageView(vulkanLogicalDevice, swapChainDepthView, nullptr);
            swapChainDepthView = nullptr;
        }

            // Color Attachments
        for(VkImageView& imageView : swapChain.imageViews)
        {
            if(imageView)
            {
                vkDestroyImageView(vulkanLogicalDevice, imageView, nullptr);
                imageView = nullptr;
            }
        }

        if(swapChain.handle)
        {
            vkDestroySwapchainKHR(vulkanLogicalDevice, swapChain.handle, nullptr);
            swapChain.handle = nullptr;
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

        InitializeSwapChain();
        CreateDepthResource();
        CreateVulkanFramebuffersForSwapchain();

        InitializeMSAAResources();

        LogSuccess("Vulkan: [Success] Swap Chain Recreated Successfully.");
    }

    /**
     * @brief UpdateUniformBuffer()
     */
    static void UpdateUniformBuffer(uint32_t currentImage)
    {
        // code
        static float totalTime = 0.0f;

        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float deltaTime = (time - totalTime) * 0.01f;
        totalTime = time;

        // Update Compute Uniform Buffer
        ComputePipelineUniformBufferObject computeUbo{};
        computeUbo.DeltaTime = deltaTime;
        computeUbo.Time = totalTime;
        computeUbo.MinAABB = MIN_AABB;
        computeUbo.MaxAABB = MAX_AABB;

        memcpy(computePipeline.uniformBuffer.mapped, &computeUbo, sizeof(computeUbo));

        // Update Graphics Uniform Buffer
        float cameraRotationAngle = glm::pi<float>() * 0.30f * time;

        GraphicsPipelineUniformBufferObject graphicUbo{};
        graphicUbo.ProjMatrix = glm::perspective(glm::radians(60.0f), swapChain.extent.width / (float)swapChain.extent.height, 0.1f, 100.0f);
        //     // GLM was originally designed for OpenGL, where teh U coordinate of the clip coordinates is inverted. The easiest way to
        //     // compendate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix.
        graphicUbo.ProjMatrix[1][1] *= -1;

        glm::vec3 cameraPosition = glm::vec3(cameraDistance * sinf(cameraRotationAngle), cameraDistance * sinf(cameraRotationAngle * 0.75f), cameraDistance * cosf(cameraRotationAngle));
        graphicUbo.ViewMatrix = glm::lookAt( cameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        memcpy(graphicsPipeline.uniformBuffer.mapped, &graphicUbo, sizeof(graphicUbo));
    }

    /**
     * @brief UpdateUI()
     */
    void UpdateUI()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();
        {
            ImGui::Begin("Vulkan VX-Shader Studio", nullptr, 0);
            {
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::Separator();
                ImGui::Separator();

                ImGui::Text("Swap Chain Settings");
                ImGui::Separator();
                ImGui::Text("Format: %s", VX_FRAMEWORK_NAMESPACE::helper::GetVkFormatString(swapChain.imageFormat));
                ImGui::Text("Present Mode: %s", VX_FRAMEWORK_NAMESPACE::helper::GetVkPresentModeString(swapChain.presentMode));
                ImGui::Text("Image Count: %d", swapChain.images.size());
                ImGui::Text("Image Extent: %d x %d", swapChain.extent.width, swapChain.extent.height);

                ImGui::Separator();
                if(ImGui::Checkbox("Enable MSAA (4x)", &msaa.requestedMSAA))
                {
                    if(msaa.requestedMSAA != msaa.useMSAA)
                    {
                        LogDebug("MSAA Checkbox Toggled: %s", msaa.useMSAA ? "Checked" : "Unchecked");
                        reInitImGui = true;
                    }
                }

                ImGui::Separator();
                ImGui::DragFloat("Camera Distance", &cameraDistance, 0.1f, -100.0f, 100.0f);
            }
            ImGui::End();
        }
        ImGui::EndFrame();

        ImGui::Render();
    }

    /**
     * @brief DrawFrame()
     */
    void DrawFrame()
    {
        if(swapChain.framebufferResized)
        {
            swapChain.framebufferResized = false;
            RecreateSwapChain();
            return;
        }

        if(reInitImGui)
        {
            vkDeviceWaitIdle(vulkanLogicalDevice);

                // Shutdown ImGui Vulkan
            ImGui_ImplVulkan_Shutdown();

            // --- APPLY THE STATE HERE ---
            // This ensures the rest of the frame (RenderPass selection, etc.) 
            // perfectly matches the new ImGui initialization.
            msaa.useMSAA = msaa.requestedMSAA;

                // Initialize ImGui Vulkan
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.handle, vulkanSurface, &surfaceCapabilities);

            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.ApiVersion             = vkInstanceVersion.PackedVersion;
            initInfo.Instance               = vkInstance;
            initInfo.PhysicalDevice         = physicalDevice.handle;
            initInfo.Device                 = vulkanLogicalDevice;
            initInfo.QueueFamily            = vulkanSelectedPhysicalDeviceQueueFamily.graphicsComputeFamily;
            initInfo.Queue                  = vulkanGraphicsComputeQueue;
            initInfo.DescriptorPool         = imgui_renderer.descriptorPool;
            initInfo.MinImageCount          = surfaceCapabilities.minImageCount;
            initInfo.ImageCount             = static_cast<uint32_t>(swapChain.images.size());
            initInfo.PipelineCache          = VK_NULL_HANDLE;

            initInfo.UseDynamicRendering    = VK_FALSE;   // We are using Render Passes, so this should be false.
            
            initInfo.PipelineInfoMain.RenderPass    = msaa.useMSAA ? msaa.renderPass_MSAA : vulkanRenderPass;
            initInfo.PipelineInfoMain.Subpass       = 0;
            initInfo.PipelineInfoMain.MSAASamples   =  msaa.useMSAA ? msaaSamplesCount : VK_SAMPLE_COUNT_1_BIT;

            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType                     = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pNext                     = nullptr;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.viewMask                  = 0;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount      = 1;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats   = &swapChain.imageFormat;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat     = VK_FORMAT_UNDEFINED;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.stencilAttachmentFormat   = VK_FORMAT_UNDEFINED;

            initInfo.Allocator = nullptr;
            initInfo.CheckVkResultFn = __check_imgui_vk_result_callback_;

            if(!ImGui_ImplVulkan_Init(&initInfo))
            {
                LogError("Failed to initialize ImGui Vulkan Backend.");
            }

            reInitImGui = false;
        }

            // Wait for the GPU to Finish
        vkWaitForFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
            // Acquire an image from the swap chain.
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR( vulkanLogicalDevice, swapChain.handle, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LogError("Failed to acquire swap chain image!!!");
            return;
        }

        vkResetFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame]);

        computePingPongIndex = frameNumber % 2;
        graphicsPingPongIndex = (frameNumber + 1) % 2;
        // LogWarning("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< START NEW FRAME >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        // LogWarning("FrameNumber: %u, CurrentFrame: %u, ComputeIndex: %d, GraphicsIndex: %d", frameNumber, currentFrame, computePingPongIndex, graphicsPingPongIndex);

        // =====================================
        // Update Uniforms/UI
        // =====================================
        UpdateUniformBuffer(currentFrame);
        UpdateUI();

        // =====================================
        // BEGIN COMMAND BUFFER
        // =====================================
        VkCommandBuffer cmdBuffer = vulkanCommandBuffers[currentFrame];
        vkResetCommandBuffer( cmdBuffer, 0);  // Make sure it is able to be recorded.

        VkCommandBufferBeginInfo beginInfo = VX_FRAMEWORK_NAMESPACE::initializers::commandBufferBeginInfo();
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkBeginCommandBuffer( cmdBuffer, &beginInfo), );

        // =====================================
        // COMPUTE PASS
        // =====================================
        vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.pipeline);
        vkCmdBindDescriptorSets( cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.pipelineLayout, 0, 1, &computePipeline.descriptorSets[computePingPongIndex], 0, nullptr);
        vkCmdDispatch( cmdBuffer, MAX_PARTICLES, 1, 1);

        // =====================================
        // THE BARRIER
        // =====================================
        VkBufferMemoryBarrier bufferBarrier = VX_FRAMEWORK_NAMESPACE::initializers::bufferMemoryBarrier();
        bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            // The barrier applies to the buffer we just WRITE to (the output buffer)
        bufferBarrier.buffer = (computePingPongIndex == 0) ? particleSSBOs[1].handle : particleSSBOs[0].handle;
        bufferBarrier.offset = 0;
        bufferBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier( cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);

        // =====================================
        // GRAPHICS PASS
        // =====================================
        VkClearValue clearValues[] = {
            {0.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0U}
        };

            // Brgin Render Pass
        VkRenderPassBeginInfo renderPassInfo = VX_FRAMEWORK_NAMESPACE::initializers::renderPassBeginInfo();
        renderPassInfo.renderPass        = msaa.useMSAA ? msaa.renderPass_MSAA : vulkanRenderPass;
        renderPassInfo.framebuffer       = msaa.useMSAA ? msaa.framebuffers_MSAA[imageIndex] : swapChain.framebuffers[imageIndex];   // framebuffer for the swapchain image we want to draw to.
        renderPassInfo.renderArea.offset = { 0, 0};
        renderPassInfo.renderArea.extent = swapChain.extent;
        renderPassInfo.clearValueCount   = 2;
        renderPassInfo.pClearValues      = clearValues;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // We already specify viewport and scissor state for this pipeline to be dyanmic. So we need to set them in command buffer
            VkViewport viewport = VX_FRAMEWORK_NAMESPACE::initializers::viewport(0.0f, 0.0f, (float)swapChain.extent.width, (float)swapChain.extent.height, 0.0f, 1.0f);
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

            VkRect2D scissor = VX_FRAMEWORK_NAMESPACE::initializers::rect2D(0, 0, swapChain.extent.width, swapChain.extent.height);
            vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, msaa.useMSAA ? graphicsPipeline.pipeline_MSAA : graphicsPipeline.pipeline);
            vkCmdBindDescriptorSets( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1, &graphicsPipeline.descriptorSets[graphicsPingPongIndex], 0, nullptr);

            vkCmdDraw(cmdBuffer, MAX_PARTICLES, 1, 0, 0);

            // Render ImGui
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
        }
            // End Render Pass
        vkCmdEndRenderPass(cmdBuffer);

        // =====================================
        // END COMMAND BUFFER
        // =====================================
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkEndCommandBuffer(cmdBuffer), );
        
        // =====================================
        // SUBMISSION
        // =====================================
        VkSubmitInfo submitInfo = VX_FRAMEWORK_NAMESPACE::initializers::submitInfo();
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmdBuffer;
            
            // We must wait for the PREVIOUS frame's compute to finish if we are doing consecutive physics steps
        VkSemaphore waitSemaphores[]      = { imageAvailableSemaphores[currentFrame], computePipelineFinishedSemaphore[(currentFrame + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
        submitInfo.waitSemaphoreCount   = (frameNumber == 0) ? 1 : 2;   // Don';'t wait on compute for the very first frame
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;

            // Signal that THIS frame's compute and rendering are done
        VkSemaphore signalSemaphores[]  = { renderFinishedSemaphores[currentFrame], computePipelineFinishedSemaphore[currentFrame] };
        submitInfo.signalSemaphoreCount = 2;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VK_UTIL_FN_ERROR_CHECK_RETURN( vkQueueSubmit( vulkanGraphicsComputeQueue, 1, &submitInfo, inFlightFences[currentFrame]), );
        // VK_UTIL_FN_ERROR_CHECK_RETURN( vkQueueSubmit( vulkanGraphicsComputeQueue, 1, &submitInfo, VK_NULL_HANDLE), );

        // =====================================
        // PRESENTATION
        // =====================================
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = signalSemaphores;
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = &swapChain.handle;
        presentInfo.pImageIndices       = &imageIndex;
        presentInfo.pResults            = nullptr;

        result = vkQueuePresentKHR(vulkanPresentationQueue, &presentInfo);

            // Check if swap chain has become incompatible with the surface and can no longer be used for rendering. (Usually happen for resize).
        if(swapChain.framebufferResized)
        {
            swapChain.framebufferResized = false;
            RecreateSwapChain();
            // DO NOT RETURN HERE. Let the frame advance.
        }
        else if(result != VK_SUCCESS)
        {
            LogError("Failed to present swap chain image!!!");
            // It's safe to throw or handle the fatal error here
        }

        // =====================================
        // Advance to the next frame
        // =====================================
        ++frameNumber;
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    /**
     * @brief Unintialize()
     */
    void Uninitialize()
    {
        // code
            // We have to wait until the device is idle before we can start destroying resources, because some of them may still be in use.
        if(vulkanLogicalDevice)
        {
            vkDeviceWaitIdle(vulkanLogicalDevice);
        }

        // Destroy ImGui
        ImGuiDestroy();

        // Swapchain cleanup
        CleanupSwapChain();
        InitializeMSAAResources(false);

        // Destroy Graphics Pipeline
        graphicsPipeline.uniformBuffer.Destroy(vulkanLogicalDevice);

        if(graphicsPipeline.descriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(vulkanLogicalDevice, graphicsPipeline.descriptorSetLayout, nullptr);
            graphicsPipeline.descriptorSetLayout = nullptr;    
        }

        if(graphicsPipeline.pipelineLayout)
        {
            vkDestroyPipelineLayout(vulkanLogicalDevice, graphicsPipeline.pipelineLayout, nullptr);
            graphicsPipeline.pipelineLayout = nullptr;
        }

        if(graphicsPipeline.pipeline)
        {
            vkDestroyPipeline(vulkanLogicalDevice, graphicsPipeline.pipeline, nullptr);
            graphicsPipeline.pipeline = nullptr;
        }

        if(graphicsPipeline.pipeline_MSAA)
        {
            vkDestroyPipeline(vulkanLogicalDevice, graphicsPipeline.pipeline_MSAA, nullptr);
            graphicsPipeline.pipeline_MSAA = nullptr;
        }

        // Destroy Compute Pipeline
        computePipeline.uniformBuffer.Destroy(vulkanLogicalDevice);

        if(computePipeline.descriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(vulkanLogicalDevice, computePipeline.descriptorSetLayout, nullptr);
            computePipeline.descriptorSetLayout = nullptr;    
        }

        if(computePipeline.pipelineLayout)
        {
            vkDestroyPipelineLayout(vulkanLogicalDevice, computePipeline.pipelineLayout, nullptr);
            computePipeline.pipelineLayout = nullptr;
        }

        if(computePipeline.pipeline)
        {
            vkDestroyPipeline(vulkanLogicalDevice, computePipeline.pipeline, nullptr);
            computePipeline.pipeline = nullptr;
        }

        if(computePipelineFinishedSemaphore[0])
        {
            vkDestroySemaphore(vulkanLogicalDevice, computePipelineFinishedSemaphore[0], nullptr);
            computePipelineFinishedSemaphore[0] = nullptr;
            vkDestroySemaphore(vulkanLogicalDevice, computePipelineFinishedSemaphore[1], nullptr);
            computePipelineFinishedSemaphore[1] = nullptr;
        }

        ////////////////////
        particleSSBOs[0].Destroy(vulkanLogicalDevice);
        particleSSBOs[1].Destroy(vulkanLogicalDevice);

        if(vulkanDescriptorPool)
        {
            vkDestroyDescriptorPool(vulkanLogicalDevice, vulkanDescriptorPool, nullptr);
            vulkanDescriptorPool = nullptr;
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
            vkFreeCommandBuffers(vulkanLogicalDevice, vulkanCommandPool, MAX_FRAMES_IN_FLIGHT, vulkanCommandBuffers);

            for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                vulkanCommandBuffers[i] = nullptr;
            }
        }
        
        if(vulkanCommandPool)
        {
            vkDestroyCommandPool(vulkanLogicalDevice, vulkanCommandPool, nullptr);
            vulkanCommandPool = nullptr;
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

        VX_FRAMEWORK_NAMESPACE::utils::DestroyVulkanInstance(&vkInstance);
    }
    
} // namespace VkApplication
