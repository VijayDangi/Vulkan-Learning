#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR   // Used to include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <cstdint>
#include <limits>       // std::numeric_limits
#include <algorithm>    // for std::clamp()

#include <string>
#include <cstring>
#include <fstream>

#include "../Common/glm/glm.hpp"
#include "../Common/glm/gtc/matrix_transform.hpp"

#include <chrono>

#include "Common.h"
#include "VkApplication.h"
#include "VulkanHelper.h"

#define ENABLE_VALIDATION_LAYER 1

namespace VkApplication
{
#ifndef MIN
    #define MIN(a, b) ( (a) > (b) ? (b) : (a))
#endif

#ifndef MAX
    #define MAX(a, b) ( (a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
    #define CLAMP(_value, _min, _max)  MIN( MAX((_value), (_min)), (_max))
#endif

    // Type Declaration
#define INVALID_QUEUE_FAMILY_HANDLE -1
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

    struct Vertex
    {
        glm::vec2 position;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            // code
            // Vertex binding describes at which rate to load data from memory throughout the vertices.
            // It specifies the number of bytes between data entries and whether to move to the next data after
            // each vertex or after each instance.
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;                                 // Specifies the index of the binding in the array of bindings.
            bindingDescription.stride = sizeof(Vertex);                     // Specifies the number of bytes from one entry to the next.
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;     // PerVertex Or PerInstance.

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
        {
            // code
            std::array<VkVertexInputAttributeDescription, 1> attributeDescription{};

            // position
            attributeDescription[0].binding = 0;
            attributeDescription[0].location = 0;   // Shader Input Location Attribute.
            attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescription[0].offset = offsetof(Vertex, position);

            return attributeDescription;
        }
    };

    
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
    struct UniformBufferObject
    {
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    struct PushConstant
    {
        glm::vec4 position_offset;
        glm::vec4 color;
    };

    // Variable Declaration
    std::vector<VkLayerProperties> vulkanAvailableLayerProperties;
    std::vector<VkExtensionProperties> vulkanAvailableExtensionProperties;

    std::vector<const char*> vulkanAvailableLayerNames;
    std::vector<const char*> vulkanAvailableExtensionNames;

    const std::vector<const char*> vulkanRequiredValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> vulkanRequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#if ENABLE_VALIDATION_LAYER
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    const int MAX_FRAMES_IN_FLIGHT = 2; // Defines how many frames should be processed concurrently.
    uint32_t currentFrame = 0;  // Track of the current frame.

    // Vulkan Specific Variables
    VkInstance vulkanInstance;
    VkDebugUtilsMessengerEXT vulkanDebugMessenger;
    VkPhysicalDevice vulkanPhysicalDevice = VK_NULL_HANDLE; // This object will be implicitly destroyedd when 'VkInstance' is destroyed.
    VkDevice vulkanLogicalDevice;  // Vulkan Logical Device
    VkSurfaceKHR vulkanSurface;
    VkSwapchainKHR vulkanSwapChain;

    VkQueue vulkanGraphicsQueue;        // Devices queue are implicitly cleaned up when device is destroyed.
    VkQueue vulkanPresentationQueue;    // Devices queue are implicitly cleaned up when device is destroyed.
    VkQueue vulkanTransferQueue;        // Devices queue are implicitly cleaned up when device is destroyed.

    std::vector<VkImage> vulkanSwapChainImages; // The images were created by the implementation for the swap chain and they
                                                // will be automatically cleaned up once the swap chain has been destroyed.
    std::vector<VkImageView> vulkanSwapChainImageViews; // Unlike VkImage, VkImageView were explicitly created by us, so need to destroy them.

    VkFormat vulkanSwapChainImageFormat;
    VkExtent2D vulkanSwapChainExtent;

    VkRenderPass vulkanRenderPass;
    VkDescriptorSetLayout vulkanDescriptorSetLayout;
    VkPipelineLayout vulkanPipelineLayout;
    VkPipeline vulkanGraphicsPipeline;

    std::vector<VkFramebuffer> vulkanSwapchainFramebuffers;

    VkCommandPool vulkanGraphicsCommandPool;
    VkCommandPool vulkanTransferCommandPool;
    VkCommandBuffer vulkanCommandBuffers[MAX_FRAMES_IN_FLIGHT];    // Command buffers will be automatically freed when their command pool is destroyed, so we don't need explicit clenup.

    // Synchromization Objects
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];    // To signal that an image has been aquired from the swapchain and is ready for rendering.
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];    // To signal that rendering has finished and presentation can happen.
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];                  // To make sure only one frame is rendering at a time.

    bool framebufferResized = false;

    // Vertex Buffer
    VkBuffer vulkanVertexBuffer;
    VkDeviceMemory vulkanVertexBufferMemory;

    // Index Buffer
    VkBuffer vulkanIndexBuffer;
    VkDeviceMemory vulkanIndexBufferMemory;

    // Uniform Buffers
    VkBuffer vulkanUniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory vulkanUniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
    void* uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT]{};

    VkDescriptorPool vulkanDescriptorPool;
    VkDescriptorSet vulkanDescriptorSets[MAX_FRAMES_IN_FLIGHT]; // Don't need to explicitly cleanup descriptor sets, becaise it will auto freed when desciptor pool destroyed.

    const std::vector<Vertex> vertices =
    {
      {{ -0.5f, -0.5f}},
      {{  0.5f, -0.5f}},
      {{  0.5f,  0.5f}},
      {{ -0.5f,  0.5f}}
    };

    const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

    struct PushConstant rectPushConstants[] = {
        {{ -4.00f,  3.00f, 0.00f, 0.00f}, { 1.00f, 1.00f, 1.00f, 1.00f}},
        {{ -1.50f,  3.00f, 0.00f, 0.00f}, { 1.00f, 0.00f, 0.00f, 1.00f}},
        {{  1.50f,  3.00f, 0.00f, 0.00f}, { 0.00f, 1.00f, 0.00f, 1.00f}},
        {{  4.00f,  3.00f, 0.00f, 0.00f}, { 0.00f, 0.00f, 1.00f, 1.00f}},

        {{ -4.00f,  1.00f, 0.00f, 0.00f}, { 0.75f, 0.75f, 0.75f, 0.75f}},
        {{ -1.50f,  1.00f, 0.00f, 0.00f}, { 0.75f, 0.00f, 0.00f, 0.75f}},
        {{  1.50f,  1.00f, 0.00f, 0.00f}, { 0.00f, 0.75f, 0.00f, 0.75f}},
        {{  4.00f,  1.00f, 0.00f, 0.00f}, { 0.00f, 0.00f, 0.75f, 0.75f}},

        {{ -4.00f, -1.00f, 0.00f, 0.00f}, { 0.50f, 0.50f, 0.50f, 0.50f}},
        {{ -1.50f, -1.00f, 0.00f, 0.00f}, { 0.50f, 0.00f, 0.00f, 0.50f}},
        {{  1.50f, -1.00f, 0.00f, 0.00f}, { 0.00f, 0.50f, 0.00f, 0.50f}},
        {{  4.00f, -1.00f, 0.00f, 0.00f}, { 0.00f, 0.00f, 0.50f, 0.50f}},

        {{ -4.00f, -3.00f, 0.00f, 0.00f}, { 0.25f, 0.25f, 0.25f, 0.25f}},
        {{ -1.50f, -3.00f, 0.00f, 0.00f}, { 0.25f, 0.00f, 0.00f, 0.25f}},
        {{  1.50f, -3.00f, 0.00f, 0.00f}, { 0.00f, 0.25f, 0.00f, 0.25f}},
        {{  4.00f, -3.00f, 0.00f, 0.00f}, { 0.00f, 0.00f, 0.25f, 0.25f}}
    };

    /**
     * @brief LogSwapChainSupportDetails()
     */
    static void LogSwapChainSupportDetails(SwapChainSupportDetails& details)
    {
        // code
        std::string log_string;

        // Capabilities
        log_string += "Swap Chain Capabilities: \n";
        log_string = log_string + "\tMinimum Image Count       :- "  +  std::to_string(details.capabilities.minImageCount) + "\n";
        log_string = log_string + "\tMaximum Image Count       :- "  +  std::to_string(details.capabilities.maxImageCount) + "\n";
        log_string = log_string + "\tCurrent Extent            :- [" +  std::to_string(details.capabilities.currentExtent.width) + ", " +
                                                                         std::to_string(details.capabilities.currentExtent.height) + "]\n";
        log_string = log_string + "\tMin Image Extent          :- ["  +  std::to_string(details.capabilities.minImageExtent.width) + ", " +
                                                                         std::to_string(details.capabilities.minImageExtent.height) + "]\n";
        log_string = log_string + "\tMax Image Extent          :- ["  +  std::to_string(details.capabilities.maxImageExtent.width) + ", " +
                                                                         std::to_string(details.capabilities.maxImageExtent.height) + "]\n";
        log_string = log_string + "\tMax Image Array Layers    :- "   +  std::to_string(details.capabilities.maxImageArrayLayers) + "\n";
        log_string = log_string + "\tSupported Transforms      :- \n"   +  VulkanHelper::GetVkSurfaceTransformsFlagString(details.capabilities.supportedTransforms) + "\n";
        log_string = log_string + "\tCurrent Transforms        :- \n"   +  VulkanHelper::GetVkSurfaceTransformsFlagString(details.capabilities.currentTransform) + "\n";
        log_string = log_string + "\tSupported Composite Alpha :- \n"   +  VulkanHelper::GetVkCompositeAlphaFlagString(details.capabilities.supportedCompositeAlpha) + "\n";
        log_string = log_string + "\tSupported Image Usage     :- \n"   +  VulkanHelper::GetVkImageUsageFlagsString(details.capabilities.supportedUsageFlags) + "\n";

        // Formats
        log_string = log_string + "\n";
        log_string = log_string + "\tSurface Formats    :-\n";
        int index = 0;
        for(VkSurfaceFormatKHR& format : details.formats)
        {
            log_string = log_string + "\t" + std::to_string(index);
            log_string = log_string + "\tFormat     = " + VulkanHelper::GetVkFormatString(format.format) + "\n";
            log_string = log_string + "\t\tColorSpace = " + VulkanHelper::GetVkColorSpaceString(format.colorSpace) + "\n\n";

            index++;
        }

        // Present Mode
        log_string = log_string + "\n";
        log_string = log_string + "\tPresent Modes    :-\n";
        for(VkPresentModeKHR& presentMode : details.presentModes)
        {
            log_string = log_string + "\t\t" + VulkanHelper::GetVkPresentModeString(presentMode) + "\n";
        }

        Log("Swap Chain Details: \n%s", log_string.c_str());
    }

    /**
     * @brief GetVulkanSupportedInstanceExtensions()
     *          Get Vulkan instance supported extensions.
     */
    static void GetVulkanSupportedInstanceExtensions()
    {
        // Get Instance Extension Properties Count.
        uint32_t vulkanExtensionCount = 0;
        VkResult vulkanErrorCode = vkEnumerateInstanceExtensionProperties( nullptr, &vulkanExtensionCount, nullptr);
        if(vulkanErrorCode)
        {
            LogError("Vulkan: Failed to query instance extension properties count: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return;
        }

        // Get Instance Extension Properties.
        vulkanAvailableExtensionProperties.resize(vulkanExtensionCount);

        vulkanErrorCode = vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vulkanAvailableExtensionProperties.data());
        if(vulkanErrorCode)
        {
            LogError("Vulkan: Failed to query instance extensions properties : %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return;
        }

        vulkanAvailableExtensionProperties.resize(vulkanExtensionCount);

        std::string out_string = "\n";
        out_string += "Vulkan Instances Extensions Properties: \n";
        for(uint32_t i = 0; i < vulkanExtensionCount; ++i)
        {
            out_string += "\t\t";
            out_string += vulkanAvailableExtensionProperties[i].extensionName;
            out_string += ",\n";

            vulkanAvailableExtensionNames.push_back( vulkanAvailableExtensionProperties[i].extensionName);
        }

        Log("Vulkan Supported Extension Count: %u", vulkanExtensionCount);
        Log("%s", out_string.c_str());
    }

    /**
     * @brief CheckValidationLayerSupport()
     *          Retrieve and check vulkan layer extension support or not.
     */
    static bool CheckValidationLayerSupport()
    {
        // code
        if(vulkanAvailableLayerProperties.size() == 0)
        {
            uint32_t layerCount = 0;
            VkResult vulkanErrorCode = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            if(vulkanErrorCode)
            {
                LogError("Vulkan: Failed to query instance layer properties count: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
                return false;
            }

            vulkanAvailableLayerProperties.resize(layerCount);
            vulkanErrorCode = vkEnumerateInstanceLayerProperties(&layerCount, vulkanAvailableLayerProperties.data());
            if(vulkanErrorCode)
            {
                LogError("Vulkan: Failed to query instance layer properties: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
                return false;
            }

            std::string out_string = "\n";
            out_string += "Vulkan Instance Layer Properties: \n";
            for(uint32_t i = 0; i < layerCount; ++i)
            {
                out_string = out_string + "\t\t" + "LayerName: " + vulkanAvailableLayerProperties[i].layerName + "\n";
                out_string = out_string + "\t\t" + "Description: " + vulkanAvailableLayerProperties[i].description + "\n";
                out_string += "\n";

                vulkanAvailableLayerNames.push_back( vulkanAvailableLayerProperties[i].layerName);
            }

            Log("Vulkan Supported Layer Count: %u", layerCount);
            Log("%s", out_string.c_str());
        }

        // Check if all of the layers in 'vulkanRequiredValidationLayers' exist in the 'vulkanAvailableLayerProperties' list.
        for(const char *layerName : vulkanRequiredValidationLayers)
        {
            bool layerFound = false;

            for(const VkLayerProperties& layerProperties : vulkanAvailableLayerProperties)
            {
                if(strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound)
            {
                return false;
            }
        }

        return true;
    }


    /**
     * @brief VulkanDebugCallback()
     *              Vulkan debug call signature => PFN_vkDebugUtilsMessengerCallbackEXT
     * 
     * @return:
     *      The callback returns a boolean that indicated if the Vulkan call that triggered the validation layer message
     *      should be aborted.
     *      If returns true, then the call is aborted with the 'VK_ERROR_VALIDATION_FAILED_EXT' error. this is normally used
     *          to test the validation layers themselves.
     */
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,     // Specified severity of the message.
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    )
    {
        // code
            // Severity:
            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:    Diagnostic Message.
            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:       Informational message like the creation of a resource.
            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:    Message about behavior that is not necessarily an error,
            //                                                              but very likely a bug in your application.
            //      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:      Message about behavior that is invalid and may cause crashes.

            // Message Type:
            //      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:                    Some event has happened that is unrelated to the specification or performance.
            //      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATIONL_BIT_EXT:                Something has happened that violated the specification or indicates a possible mistake.
            //      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:                Potential non-optimal use of Vulkan.
            //      VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:     The implementation has modified the set of GPU-visible virtual addresses associated with a Vulkan object.
        switch(messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                Log("Validation Layer: %s", pCallbackData->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                LogInfo("Validation Layer: %s", pCallbackData->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                LogWarning("Validation Layer: %s", pCallbackData->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                LogError("Validation Layer: %s", pCallbackData->pMessage);
                break;
        }

        return VK_FALSE;
    }

    /**
     * @brief CreateDebugUtilsMessengerEXT()
     */
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger
    )
    {
        // code
        // The above struct have to passed to vkCreateDebugUtilsMessagerEXT() function to create VkDebugUtilsMessengerEXT object.
        // Unfortunatly, because this function is an extension function, it is not automatically loaded.
        PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if(func)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    /**
     * @brief DestroyDebugUtilsMessengerEXT()
     */
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        // code
        // This function is an extension function, so it is needed to be explicitly loaded.
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if(func)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    /**
     * @brief PopulateDebugMessengerCreateInfo()
     */
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        // code
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

        createInfo.pfnUserCallback = VulkanDebugCallback;
        createInfo.pUserData = nullptr;
    }

    /**
     * @brief CreateVulkanInstance() :
     *              Initialize the Vulkan Library by creating an instance.
     *              The instance is the connection between our application and the vulkan library and 
     *              creating it involves specifying some details about our application to the driver.
     */
    static bool CreateVulkanInstance()
    {
        // code
        VkResult vulkanErrorCode;

        ////////////////////////////////////
        if(enableValidationLayers && !CheckValidationLayerSupport())
        {
            LogError("Vulkan: validation layers requested, but not available!!!");
            return false;
        }

        /*
            typedef struct VkApplicationInfo {
                VkStructureType    sType;
                const void*        pNext;
                const char*        pApplicationName;
                uint32_t           applicationVersion;
                const char*        pEngineName;
                uint32_t           engineVersion;
                uint32_t           apiVersion;
            } VkApplicationInfo;
        */
        VkApplicationInfo appInfo{};                            // Optional Struct
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.pNext = nullptr;

        /*
            typedef struct VkInstanceCreateInfo {
                VkStructureType             sType;
                const void*                 pNext;
                VkInstanceCreateFlags       flags;
                const VkApplicationInfo*    pApplicationInfo;
                uint32_t                    enabledLayerCount;
                const char* const*          ppEnabledLayerNames;
                uint32_t                    enabledExtensionCount;
                const char* const*          ppEnabledExtensionNames;
            } VkInstanceCreateInfo;
        */
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

            // Required Extensions which we want to enabled.
        std::vector<const char *> requiredExtensions;

        requiredExtensions.emplace_back("VK_KHR_surface");
        requiredExtensions.emplace_back("VK_KHR_win32_surface");

            // To set up callback in the program to handle messages and the associated details, we have to set
            // up a debug messenger with a callback using the "VK_EXT_debug_utils" extension.
        if(enableValidationLayers)
        {
            requiredExtensions.emplace_back("VK_EXT_debug_utils");
        }

        createInfo.enabledExtensionCount = requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // Placed this variable outside of if statement to ensure that it is not destroyed before the 'vkCreateInstance()' call.
        if(enableValidationLayers)
        {
            // Enable Validation Layer.
            createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanRequiredValidationLayers.size());
            createInfo.ppEnabledLayerNames = vulkanRequiredValidationLayers.data();

            // To create a separate debug utils messenger specifically for 'vkCreateInstace()' and 'vkDestroyInstance()'
            // function calls. It requires you to simply pass a pointer to a 'vkDebugUtilsMessengerCreateInfoEXT' struct
            // in the 'pNext' extension field of 'vkInstanceCreateInfo()'.
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        vulkanErrorCode = vkCreateInstance(&createInfo, nullptr, &vulkanInstance);
        if(vulkanErrorCode != VK_SUCCESS)
        {
            LogError("vkCreateInstance() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return false;
        }
        else
        {
            LogSuccess("vkCreateInstance() Success with Validation %s.", enableValidationLayers ? "Enabled" : "Disabled");
        }

        return true;
    }

    /**
     * @brief SetupVulkanDebugMessenger() :
     *              If validation layer enabled, setup vulkan debug callback.
     */
    static bool SetupVulkanDebugMessenger()
    {
        // code
        if(!enableValidationLayers)
        {
            return true;    // return true, even if validation not enabled, so that execution want stop.
        }

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);

        VkResult errorCode = CreateDebugUtilsMessengerEXT(vulkanInstance, &debugMessengerCreateInfo, nullptr, &vulkanDebugMessenger);
        if(errorCode != VK_SUCCESS)
        {
            LogError("vkCreateDebugUtilsMessengerEXT() Failed: %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        return true;
    }

    /**
     * @brief CreateVulkanSurface()
     */
    static bool CreateVulkanSurface(HWND hwnd)
    {
        // code
        VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo{};
        win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32SurfaceCreateInfo.hwnd = hwnd;
        win32SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

        VkResult errorCode = vkCreateWin32SurfaceKHR(vulkanInstance, &win32SurfaceCreateInfo, nullptr, &vulkanSurface);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkam: [Error] vkCreateWin32SurfaceKHR() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        LogSuccess("Vulkan: [Success] Vulkan Surface Create Successfully.");

        return true;
    }

    /**
     * @brief FindQueueFamilies()
     */
    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
    {
        // code
        QueueFamilyIndices queueIndices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for(const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            // Check if Device has Graphics queue, .
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueIndices.graphicsFamily = i;
            }

            if(!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
               (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
            {
                queueIndices.transferFamily = i;
            }

            // Check if device support surface presentation.
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkanSurface, &presentSupport);
            if(presentSupport)
            {
                queueIndices.presentFamily = i;
            }

            if(queueIndices.IsComplete())
            {
                LogInfo("Graphics Queue Family Index: %u", queueIndices.graphicsFamily);
                LogInfo("Present Queue Family Index: %u", queueIndices.presentFamily);
                LogInfo("Transfer Queue Family Index: %u", queueIndices.transferFamily);
                break;
            }

            ++i;
        }

        return queueIndices;
    }

    /**
     * @brief CheckDeviceExtensionSupport()
     */
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // code
        uint32_t extensionCount = 0;
        VkResult errorCode = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkEnumerateDeviceExtensionProperties() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        errorCode = vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data());
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkEnumerateDeviceExtensionProperties() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        std::set<std::string> requiredExtensions(vulkanRequiredDeviceExtensions.begin(), vulkanRequiredDeviceExtensions.end());

        for(const VkExtensionProperties& extension : availableExtensions)
        {
            // LogInfo("%s", extension.extensionName);
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    
    /**
     * @brief QuerySwapChainSupport()
     */
    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
    {
        // code
        SwapChainSupportDetails details;

        // Query Surface Capabilities
        VkResult errorCode = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, vulkanSurface, &details.capabilities);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkGetPhysicalDeviceSurfaceCapabililtiesKHR() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return SwapChainSupportDetails();
        }

        // Querying Supported surface formats.
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, nullptr);

        if(formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, details.formats.data());
        }

        // Querying Present Modes
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR( device, vulkanSurface, &presentModeCount, nullptr);
        if(presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, details.presentModes.data());
        }

        LogSwapChainSupportDetails(details);

        return details;
    }

    /**
     * @brief IsPhysicalDeviceSuitable()
     *              Check whether device is suitable for the operations we want to perform.
     */
    static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device)
    {
        // code
#if 0
            // Get Physical Device Details.
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Let's consider our application only usable for dedicated graphics card that support geometry shaders.
        return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && deviceFeatures.geometryShader;
#else
        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool extensionSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if(extensionSupported)
        {
            // For now Swap chain support if sufficient if there is at least one supported image format
            // and one supported presentation mode given the window surface we have.
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.IsComplete() && extensionSupported && swapChainAdequate;
#endif
    }

    /**
     * @brief PickVulkanPhysicalDevice()
     */
    static bool PickVulkanPhysicalDevice()
    {
        // code
            // Listing the graphics cards.
        uint32_t deviceCount = 0;
        VkResult vulkanErrorCode = vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
        if(deviceCount == 0)
        {
            LogError("Vulkan [Error]: Failed to find GPUs with Vulkan Support!");
            return false;
        }

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vulkanErrorCode = vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, physicalDevices.data());
        if(vulkanErrorCode)
        {
            LogError("Vulkan [Error]: vkEnumeratePhysicalDevices() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return false;
        }

        // Print Device Informations
        std::string device_details_log;

        int index = 1;
        for(const VkPhysicalDevice& device : physicalDevices)
        {
            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            std::string deviceType;
            switch(deviceProperties.deviceType)
            {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:             deviceType = "OTHER";           break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:    deviceType = "INTEGRATED GPU";  break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:      deviceType = "DISCRETE GPU";    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       deviceType = "VIRTUAL GPU";     break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:               deviceType = "CPU";             break;
            }

            device_details_log += std::to_string(index) + ".\n";
            device_details_log = device_details_log + "\t\tDevice Name:    " + deviceProperties.deviceName + "\n";
            device_details_log = device_details_log + "\t\tDevice Type:    " + deviceType + "\n";

            char hexVenderID[16]{};
            sprintf(hexVenderID, "0x%X", deviceProperties.vendorID);

            char hexDeviceID[16]{};
            sprintf(hexDeviceID, "0x%X", deviceProperties.deviceID);

            device_details_log = device_details_log + "\t\tVender ID:      " + hexVenderID + "\n";
            device_details_log = device_details_log + "\t\tDevice ID:      " + hexDeviceID + "\n";
            device_details_log = device_details_log + "\t\tAPI Version:    " + 
                std::to_string(VK_API_VERSION_MAJOR(deviceProperties.apiVersion)) + "." +
                std::to_string(VK_API_VERSION_MINOR(deviceProperties.apiVersion)) + "." +
                std::to_string(VK_API_VERSION_PATCH(deviceProperties.apiVersion)) + "\n";
            device_details_log = device_details_log + "\t\tDriver Version: " +
                std::to_string( deviceProperties.driverVersion >> 22) + "." +           // Major Version
                std::to_string((deviceProperties.driverVersion >> 12) & 0x3ff) + "." +  // Minor Version
                std::to_string( deviceProperties.driverVersion & 0xfff) + "\n";         // Patch
            device_details_log += "\n";

            index++;
        }

        Log("\nPhysical Device Properties:\n%s", device_details_log.c_str());
        device_details_log.clear();

        // Pick Physical Device
        for(const VkPhysicalDevice& device : physicalDevices)
        {
            if(IsPhysicalDeviceSuitable(device))
            {
                VkPhysicalDeviceProperties deviceProperties{};
                vkGetPhysicalDeviceProperties(device, &deviceProperties);

                LogInfo("Vulkan: Selected Physical Device Name - \"%s\"", deviceProperties.deviceName);

                vulkanPhysicalDevice = device;
                break;
            }
        }

        if(vulkanPhysicalDevice == VK_NULL_HANDLE)
        {
            LogError("Vulkan [Error]: Failed to find suitable GPI!!!");
            return false;
        }

        return true;
    }

    /**
     * @brief CreateVulkanLogicalDevice()
     */
    static bool CreateVulkanLogicalDevice()
    {
        // code
        QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);

        ///////////////// Specifying the queues to be created.
#if 0
            // 'VkDeviceQueueCreateInfo' this structure describes the number of queues we want for a single family.

            // Graphics Queue
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
        graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
        graphicsQueueCreateInfo.queueCount = 1; // Number of graphics queues we want
            // vulkan lets you assign priorities to queues to influence the scheduling of command buffer execution using floating
            // point number between 0.0 and 1.0. This is required even if there is only a single queue.
        float queuePriorities = 1.0f;
        graphicsQueueCreateInfo.pQueuePriorities = &queuePriorities;

            // Presentation Queue
        VkDeviceQueueCreateInfo presentationQueueCreateInfo{};
        presentationQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentationQueueCreateInfo.queueFamilyIndex = indices.presentFamily;
        presentationQueueCreateInfo.queueCount = 1; // Number of presentation queues we want
        presentationQueueCreateInfo.pQueuePriorities = &queuePriorities;
#else
        // VkDeviceQueueCreateInfo.queueFamilyIndex must be unique
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily, indices.transferFamily};

        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }
#endif

        ///////////////// Specifying set of device features that we'll be using.
            // Right Now we don't need anythi special, so we can simply defint it and leave everyting to VK_FALSE.
        VkPhysicalDeviceFeatures deviceFeatures{};

        ///////////////// Creating the logical device
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanRequiredDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = vulkanRequiredDeviceExtensions.data();

        if(enableValidationLayers)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(vulkanAvailableLayerNames.size());
            deviceCreateInfo.ppEnabledLayerNames = vulkanAvailableLayerNames.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        VkResult errorCode = vkCreateDevice(vulkanPhysicalDevice /* Device to interface with */, &deviceCreateInfo, nullptr, &vulkanLogicalDevice);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkCreateDevice() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        ///////////////// Retrieving Queue Handles
        vkGetDeviceQueue(vulkanLogicalDevice, indices.graphicsFamily, 0, &vulkanGraphicsQueue);
        vkGetDeviceQueue(vulkanLogicalDevice, indices.presentFamily, 0, &vulkanPresentationQueue);
        vkGetDeviceQueue(vulkanLogicalDevice, indices.transferFamily, 0, &vulkanTransferQueue);

        LogSuccess("Vulkan: [Success] Vulkan Device And Queue Create Successfully.");

        return true;
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
    static bool CreateVulkanSwapChain()
    {
        // code
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(vulkanPhysicalDevice);

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
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = vulkanSurface;

            // Specify Details images
        swapChainCreateInfo.minImageCount = imageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            // Specify how to handle swap chain images that will be across multiple queue families.
        QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily};

        if(indices.graphicsFamily != indices.presentFamily)
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

        VkResult errorCode = vkCreateSwapchainKHR( vulkanLogicalDevice, &swapChainCreateInfo, nullptr, &vulkanSwapChain);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateSwapchainKHR() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        LogSuccess("Vulkan: [Success] Vulkan SwapChain Create Successfully.");

        // Retrieving the swap chain images.
        imageCount = 0;
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, vulkanSwapChain, &imageCount, nullptr);
        vulkanSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(vulkanLogicalDevice, vulkanSwapChain, &imageCount, vulkanSwapChainImages.data());

        LogSuccess("Vulkan: [Success] %u Images Retrieved from Swap Chain.", imageCount);

        // Store the format and extent we've chosen for the swap chain images.
        vulkanSwapChainImageFormat = surfaceFormat.format;
        vulkanSwapChainExtent = extent;

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

        size_t imageCount = vulkanSwapChainImages.size();
        vulkanSwapChainImageViews.resize(imageCount);

        for(size_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = vulkanSwapChainImages[imageIndex];

                // Specify how the image data should be interpreted.
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = vulkanSwapChainImageFormat;

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
            VkResult errorCode = vkCreateImageView(vulkanLogicalDevice, &imageViewCreateInfo, nullptr, &vulkanSwapChainImageViews[imageIndex]);
            if(errorCode)
            {
                LogError("Vulkan: [Error] vkCreateImageView() Failed for %d. %s", imageIndex, VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }
        }

        LogSuccess("Vulkan: [Success] VkImageView created for swap chain images.");

        return true;
    }

    /**
     * @brief ReadFile()
     */
    static bool ReadFile(const std::string& filename, /* _Out_ */ std::vector<char>& out_data)
    {
        // code
        std::ifstream file(filename, std::ios::ate /* Start reading at the end of the file */ | std::ios::binary /* Read the file as binary file */);

        if(!file.is_open())
        {
            LogError("[Error] Failed to open file '%s'.", filename.c_str());
            return false;
        }

        size_t fileSize = (size_t) file.tellg();

        out_data.clear();
        out_data.resize(fileSize);

        // seek to the beaginning of the file
        file.seekg(0);
        file.read(out_data.data(), fileSize);

        file.close();

        return true;
    }

    /**
     * @brief CreateVulkanShaderModule()
    */
    VkShaderModule CreateVulkanShaderModule(const std::vector<char>& shader_code)
    {
        // code
            // Shader modules are just a thin wrapper around the shader bytecode that we've previously loaded from a file
            // and the functions defined in it. The compilation and linking of the SPIR-V bytecode to machine code for
            // execution by the GPU doesn't happen until the graphics pipeline is created.
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = shader_code.size();

            // When we perform casting from char* to uint32_t*, we also need to ensure that the data satisfies
            // the alignment requirements of uint32_t. Luckly, the data is stored in an std::vector where the default
            // allocator already ensures that the data satisfies the worst case alignment requirements.
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());

        VkShaderModule shaderModule = nullptr;
        VkResult errorCode = vkCreateShaderModule( vulkanLogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateShaderModule() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return nullptr;
        }

        return shaderModule;
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
        colorAttachmentDescrition.format = vulkanSwapChainImageFormat;
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

        // Textures and framebuffers in Vulkan are represented by VkImage objects with a vertain pixel format,
        // however the layout of the pixels in memory can change based on what you're trying to do with an image.
        //
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :- Images used as color attachment.
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR          :- Images to be presented in the swap chain.
        // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL     :- Images to be used as destination for a memory copy operation.
        colorAttachmentDescrition.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;        // Specifies which layout the image will have before the render pass begins.
        colorAttachmentDescrition.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // Specifies the layout to automatically transition to when the render pass finishes.

////////////// Subpasses and attachment references.
        // A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the
        // contents of framebuffers in previous passes, for example a sequence of post-processing effects that are applied one after another.

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;        // Specifies which attachment to reference by its index in the attachment descriptions array.
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifies which layout we would like the attachment to have during a subpass that uses this reference.

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;

#if 0
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;

        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;

        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
#endif

////////////// Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachmentDescrition;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;

#if 0
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
#endif

        VkResult errorCode = vkCreateRenderPass( vulkanLogicalDevice, &renderPassCreateInfo, nullptr, &vulkanRenderPass);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateRenderPass() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        LogSuccess("Vulkan: [Success] Vulkan Graphics Render Pass Created.");

        return true;
    }

    /**
     * @brief CreateVulkanDescriptorSetLayout()
    */
    static bool CreateVulkanDescriptorSetLayout()
    {
        // code
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;           // Same value as in vertex shader.
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        
        VkResult errorCode = vkCreateDescriptorSetLayout( vulkanLogicalDevice, &layoutInfo, nullptr, &vulkanDescriptorSetLayout);
        if(errorCode != VK_SUCCESS)
        {
            LogError("[Error] Failed to create descriptor set layout. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        return true;
    }

    /**
     * @brief CreateVulkanGraphicsPipeline()
    */
    static bool CreateVulkanGraphicsPipeline()
    {
        // code
////////////// Loading a shader.
        std::vector<char> vertexShaderCode;
        std::vector<char> fragmentShaderCode;

        if(!ReadFile("shaders/triangle.vert.spv", vertexShaderCode) ||
           !ReadFile("shaders/triangle.frag.spv", fragmentShaderCode))
        {
            return false;
        }

////////////// Creating shader modules.
        VkShaderModule vertexShaderModule = CreateVulkanShaderModule(vertexShaderCode);
        VkShaderModule fragmentShaderModule = CreateVulkanShaderModule(fragmentShaderCode);

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

////////////// Vertex Input
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};

        VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 1> attributeDescription = Vertex::getAttributeDescriptions();

        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;

        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescription.data();

////////////// Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

////////////// Viewports and scissors
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)vulkanSwapChainExtent.width;
        viewport.height = (float)vulkanSwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0};
        scissor.extent = vulkanSwapChainExtent;

////////////// Dynamic State
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

            // This will cause the configuration of these values to be ignored and you will be able (and required) to
            // specify the data at drawing time. This results in a more flexible setup and is very common for things
            // like viewport and scissor state, which would result in a more complex setup when being backed into 
            // the pipeline state.
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;

////////////// Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo{};
        rasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerStateCreateInfo.depthClampEnable = VK_FALSE;
        
        rasterizerStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;   // if it true then geometry never passes through the rasterizer stage. (Disable any output to the framebuffer).
        
        rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;   // To use other than fill mode, we have to enable GPU feature.
        
        rasterizerStateCreateInfo.lineWidth = 1.0f;                     // To use line width grater than 1.0f, we have to enable GPU feature.
        
        rasterizerStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        rasterizerStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizerStateCreateInfo.depthBiasConstantFactor = 0.0f;   // Optional
        rasterizerStateCreateInfo.depthBiasClamp = 0.0f;   // Optional
        rasterizerStateCreateInfo.depthBiasSlopeFactor = 0.0f;   // Optional
        
////////////// Multisampling
#if 1
        // Enabling it requires enabling a GPU feature.
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.minSampleShading = 1.0f;          // Optional
        multisampleCreateInfo.pSampleMask = nullptr;            // Optional
        multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;      // Optional
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
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};    // Contains the configuration per attached framebuffer.
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                   VK_COLOR_COMPONENT_G_BIT |
                                                   VK_COLOR_COMPONENT_B_BIT |
                                                   VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;   // Optional
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;    // Optional
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;   // Optional
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;   // Optional


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
        colorBlendingCreateInfo.blendConstants[0] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[1] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[2] = 0.0f;   // Optional
        colorBlendingCreateInfo.blendConstants[3] = 0.0f;   // Optional

////////////// Pipeline Layout
        // Define the push constant range used by the pipeline layout
        // Note that the specification only requires a minimum of 128 bytes, so for passing larger blocks of data
        //  you'd use UBOs or SSBOs
        VkPushConstantRange pushConstantRange{};
            // Push Constant will only be accessible at the selected pipeline stage.
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(struct PushConstant);

        // The uniform values need to be specified during pipeline creation by creating VkPipelineLayout object.
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &vulkanDescriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;    // Optional
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange; // Optional

        VkResult errorCode = vkCreatePipelineLayout(vulkanLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &vulkanPipelineLayout);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreatePipelineLayput() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

////////////// Create Graphics Pipeline
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

            // Referencing programmable-function stage.
        graphicsPipelineCreateInfo.stageCount = 2;
        graphicsPipelineCreateInfo.pStages = shaderStages;

            // Referencing fixed-function stages.
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizerStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = nullptr;    // Optional
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

            // pipeline layout
        graphicsPipelineCreateInfo.layout = vulkanPipelineLayout;

            // Finally renference to the render pass and the index of the sub pass where this graphics pipeline will be used.
        graphicsPipelineCreateInfo.renderPass = vulkanRenderPass;
        graphicsPipelineCreateInfo.subpass = 0;

        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;

        errorCode = vkCreateGraphicsPipelines( vulkanLogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &vulkanGraphicsPipeline);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateGraphicsPipelines() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // Destroy Shader Module
        vkDestroyShaderModule( vulkanLogicalDevice, fragmentShaderModule, nullptr);
        vkDestroyShaderModule( vulkanLogicalDevice, vertexShaderModule, nullptr);

        LogSuccess("Vulkan: [Success] Vulkan Graphics Pipeline Created.");

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
        vulkanSwapchainFramebuffers.resize( vulkanSwapChainImageViews.size());

        // Iterate through the image views and create framebuffers from them:
        for(size_t i = 0; i < vulkanSwapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = { vulkanSwapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = vulkanRenderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.width = vulkanSwapChainExtent.width;
            framebufferCreateInfo.height = vulkanSwapChainExtent.height;
            framebufferCreateInfo.layers = 1;

            VkResult errorCode = vkCreateFramebuffer( vulkanLogicalDevice, &framebufferCreateInfo, nullptr, &vulkanSwapchainFramebuffers[i]);
            if(errorCode)
            {
                LogError("Vulkan: [Error] vkCreateFramebuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }
        }

        LogSuccess("Vulkan: [Success] Vulkan Swapchain Frambuffers Created.");

        return true;
    }

    /**
     * @brief CreatevulkanGraphicsCommandPool()
     */
    static bool CreatevulkanGraphicsCommandPool(VkCommandPoolCreateFlags poolCreateFlag, uint32_t queueFamilyIndex, VkCommandPool &out_commandPool)
    {
        // code
            // Command pools manages the memory that is used to store the buffers and command buffers
            // are allocated from them.
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        //
        //  VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : Hind that command buffers are rerecorded with new commands very often (may change memory allocation behaviour)
        //  VK_COMMAND_POOL_CREATE_REST_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded individually, without this flag they all have to be reset together.
        //  VK_COMMAND_POOL_CREATE_PROTECTED_BIT  : Specifies that command buffers allocated from the pool are protected command buffers.
        //
        commandPoolCreateInfo.flags = poolCreateFlag;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

        VkResult errorCode = vkCreateCommandPool(vulkanLogicalDevice, &commandPoolCreateInfo, nullptr, &out_commandPool);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateCommandPool() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        LogSuccess("Vulkan: [Success] Vulkan Command Pool Created for QueueFamilyIndex: %u.", queueFamilyIndex);

        return true;
    }

    /**
     * @brief CreateVulkanCommandBuffers()
     */
    static bool CreateVulkanCommandBuffers(VkCommandPool commandPool)
    {
        // code
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = commandPool;

        // VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
        // VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        VkResult errorCode = vkAllocateCommandBuffers( vulkanLogicalDevice, &allocateInfo, vulkanCommandBuffers);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkAllocateCommandBuffers() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        LogSuccess("Vulkan: [Success] Create Command Buffer.");
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
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;\
            // Create a fence in the signaled state, so that first call to vkWaitForFences() in DrawFrame() returns
            // immediately since the fence is already signaled.
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult errorCode;

        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            errorCode = vkCreateSemaphore(vulkanLogicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
            if(errorCode)
            {
                LogError("Vulkan: [Error] vkCreateSemaphore() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }

            errorCode = vkCreateSemaphore(vulkanLogicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
            if(errorCode)
            {
                LogError("Vulkan: [Error] vkCreateSemaphore() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }

            errorCode = vkCreateFence(vulkanLogicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]);
            if(errorCode)
            {
                LogError("Vulkan: [Error] vkCreateFence() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }
        }
        
        return true;
    }

    /**
     * @brief FindVulkanMemoryType()
     */
    static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        // code
        // Get available types of memory.
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(vulkanPhysicalDevice, &memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if( (typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        LogError("Failed To Find Suitable memory type.");
        return -1;
    }

    /**
     * @brief CreateBuffer()
     */
    static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        // code
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;         // Sizeof of buffer.
        bufferInfo.usage = usage;       // Purpose of buffer (can specify multiple purposes).
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        QueueFamilyIndices indices = FindQueueFamilies(vulkanPhysicalDevice);
        uint32_t queueFamiliesIndices[] = { indices.graphicsFamily, indices.transferFamily};
        bufferInfo.queueFamilyIndexCount = 2;
        bufferInfo.pQueueFamilyIndices = queueFamiliesIndices;

        VkResult errorCode = vkCreateBuffer( vulkanLogicalDevice, &bufferInfo, nullptr, &buffer);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkCreateBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // Memory Requirement
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vulkanLogicalDevice, buffer, &memRequirements);

        // Memory Allocation
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        uint32_t memoryType = FindMemoryType(memRequirements.memoryTypeBits, memoryProperties);
        if(memoryType == -1)
        {
            LogError("FindMemoryType() Failed.");
            return false;
        }
        allocInfo.memoryTypeIndex = memoryType;

        errorCode = vkAllocateMemory( vulkanLogicalDevice, &allocInfo, nullptr, &bufferMemory);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkAllocateBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // Associate Memory with buffer
        vkBindBufferMemory( vulkanLogicalDevice, buffer, bufferMemory, 0);

        return true;
    }

    /**
     * @brief CopyBuffer()
     */
    static void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
    {
        // code
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vulkanTransferCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vulkanLogicalDevice, &allocInfo, &commandBuffer);

        // Start Recording Command
            // We're only going to use the command buffer once and wait with returning from the function until
            // the copy operation has finished executing. It's good practice to tell the driver about our intent
            // using 'VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT'.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        // Copy Command
            // Contents of buffers are transferred using the 'vkCmdCopyBuffer()' command. It takes the source and destination
            // buffers as arguments, and an array of regions to copy.
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer( commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

        // End Recording Command
        vkEndCommandBuffer(commandBuffer);

        // Execute commands
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vulkanTransferQueue, 1, &submitInfo, VK_NULL_HANDLE);

            // Unlike the draw commands, there are no events we need to wait on this time. We just want to execute
            // the transfer on the buffers immediately.
            // There are again two possible ways to wait on this transfer to complete. We could use a fence and wait with
            // 'vkWaitForFences()', or simply wait for the transfer queue to become idle with 'vkQueueWaitIdle()'.
        vkQueueWaitIdle(vulkanTransferQueue);

        vkFreeCommandBuffers(vulkanLogicalDevice, vulkanTransferCommandPool, 1, &commandBuffer);
        commandBuffer = nullptr;
    }

    /**
     * @brief CreateVertexBuffer()
     */
    static bool CreateVertexBuffer()
    {
        // code
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        if(!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory))
        {
            LogError("[Error] CreateBuffer() Failed.");
            return false;
        }

            // Map buffer memory into CPU accessible memory
        void *data = nullptr;
        vkMapMemory(vulkanLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(vulkanLogicalDevice, stagingBufferMemory);
        data = nullptr;

        if(!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanVertexBuffer, vulkanVertexBufferMemory))
        {
            LogError("[Error] CreateBuffer() Failed.");
            return false;
        }

        // copy data from staging buffer to vertex buffer
        CopyBuffer(stagingBuffer, vulkanVertexBuffer, bufferSize);

        // Cleanup
        vkDestroyBuffer( vulkanLogicalDevice, stagingBuffer, nullptr);
        stagingBuffer = nullptr;

        vkFreeMemory(vulkanLogicalDevice, stagingBufferMemory, nullptr);
        stagingBufferMemory = nullptr;

        LogSuccess("Vulkan: [Success] Create Vertex Buffer and Memory allocated.");
        return true;
    }

    /**
     * @brief CreateIndexBuffer()
     */
    static bool CreateIndexBuffer()
    {
        // code
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        if(!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory))
        {
            LogError("[Error] CreateBuffer() Failed.");
            return false;
        }

        // Uploda Data to Staging buffer
        void *data = nullptr;
        vkMapMemory(vulkanLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(vulkanLogicalDevice, stagingBufferMemory);

        // Create Indices buffers
        if(!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanIndexBuffer, vulkanIndexBufferMemory))
        {
            LogError("[Error] CreateBuffer() Failed.");
            return false;
        }

        // Copy data from staging buffer to index buffer
        CopyBuffer(stagingBuffer, vulkanIndexBuffer, bufferSize);

        // Cleanup
        vkDestroyBuffer( vulkanLogicalDevice, stagingBuffer, nullptr);
        stagingBuffer = nullptr;

        vkFreeMemory(vulkanLogicalDevice, stagingBufferMemory, nullptr);
        stagingBufferMemory = nullptr;

        LogSuccess("Vulkan: [Success] Create Vertex Buffer and Memory allocated.");
        return true;
    }

    /**
     * @brief CreateUniformBuffer()
     */
    static bool CreateUniformBuffer()
    {
        // code
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if(!CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkanUniformBuffers[i], vulkanUniformBuffersMemory[i]))
            {
                LogError("[Error] CreateBuffer() Failed for Uniform at %d.", (int)i);
                return false;
            }

            vkMapMemory(vulkanLogicalDevice, vulkanUniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }

        return true;
    }

    /**
     * @brief CreateDescriptorPool()
     */
    static bool CreateDescriptorPool()
    {
        // code
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkResult errorCode = vkCreateDescriptorPool(vulkanLogicalDevice, &poolInfo, nullptr, &vulkanDescriptorPool);
        if(errorCode)
        {
            LogError("[Error] vkCreateDescriptorPool() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        return true;
    }

    /**
     * @brief CreateDescriptorSets()
     */
    static bool CreateDescriptorSets()
    {
        // code
        VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
        for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            layouts[i] = vulkanDescriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = vulkanDescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts;
        
        VkResult errorCode = vkAllocateDescriptorSets(vulkanLogicalDevice, &allocInfo, vulkanDescriptorSets);
        if(errorCode)
        {
            Log("vkAllocateDescriptorSets() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // Configure Desciptor
        for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vulkanUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = vulkanDescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(vulkanLogicalDevice, 1, &descriptorWrite, 0, nullptr);
        }

        return true;
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
        VkResult vulkanErrorCode;
        
            // Vulkan Supported Extensions
        GetVulkanSupportedInstanceExtensions();
        
        // Create Vulkan Instace
        CHECK_FUNCTION_RETURN(CreateVulkanInstance());

        // Setup Validation Layer
        CHECK_FUNCTION_RETURN(SetupVulkanDebugMessenger());

        // Create Surface
        CHECK_FUNCTION_RETURN(CreateVulkanSurface(hwnd));

        // Select Physical Device
        CHECK_FUNCTION_RETURN(PickVulkanPhysicalDevice());

        // Setup Logical Device
        CHECK_FUNCTION_RETURN(CreateVulkanLogicalDevice());

        // Create Swap Chain
        CHECK_FUNCTION_RETURN(CreateVulkanSwapChain());

        // Create Image Views of SwapChain images
        CHECK_FUNCTION_RETURN(CreateVulkanImageViewsForSwapchain());

        // Create Render Pass
        CHECK_FUNCTION_RETURN(CreateVulkanRenderPass());

        // Create Descriptor Set Layout
        CHECK_FUNCTION_RETURN(CreateVulkanDescriptorSetLayout());

        // Create Grpahics Pipeline
        CHECK_FUNCTION_RETURN(CreateVulkanGraphicsPipeline());

        // Create Swapchain Framebuffer
        CHECK_FUNCTION_RETURN(CreateVulkanFramebuffersForSwapchain());

        // Create Vulkan Command Pool
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(vulkanPhysicalDevice);
            // we will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
            // We're going to record commands for drawing, which is why we've chosen the graphics queue family.
        CHECK_FUNCTION_RETURN(CreatevulkanGraphicsCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndices.graphicsFamily, vulkanGraphicsCommandPool));

            // Apply memory allocation optimizations, for that use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
        CHECK_FUNCTION_RETURN(CreatevulkanGraphicsCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queueFamilyIndices.transferFamily, vulkanTransferCommandPool));

        // Create Vertex Buffer
        CHECK_FUNCTION_RETURN(CreateVertexBuffer());

        // Create Index Buffer
        CHECK_FUNCTION_RETURN(CreateIndexBuffer());

        // Create Uniform Buffer
        CHECK_FUNCTION_RETURN(CreateUniformBuffer());

        // Create Descriptor Pool
        CHECK_FUNCTION_RETURN(CreateDescriptorPool());

        // Create Descriptor Sets
        CHECK_FUNCTION_RETURN(CreateDescriptorSets());

        // Create Command Buffers
        CHECK_FUNCTION_RETURN(CreateVulkanCommandBuffers(vulkanGraphicsCommandPool));

        // Create Sync Objects
        CHECK_FUNCTION_RETURN(CreateVulkanSyncObjects());

        return true;

#undef CHECK_FUNCTION_RETURN
    }

    /**
     * @brief ResizeCallback()
     */
    void ResizeCallback(int width, int height)
    {
        // code
        framebufferResized = true;  // Handling Resize Explicitly
    }

    /**
     * @brief CleanupSwapChain()
     */
    static void CleanupSwapChain()
    {
        // code
        for(VkFramebuffer& framebuffer : vulkanSwapchainFramebuffers)
        {
            if(framebuffer)
            {
                vkDestroyFramebuffer(vulkanLogicalDevice, framebuffer, nullptr);
                framebuffer = nullptr;
            }
        }

        for(VkImageView& imageView : vulkanSwapChainImageViews)
        {
            if(imageView)
            {
                vkDestroyImageView(vulkanLogicalDevice, imageView, nullptr);
                imageView = nullptr;
            }
        }

        if(vulkanSwapChain)
        {
            vkDestroySwapchainKHR(vulkanLogicalDevice, vulkanSwapChain, nullptr);
            vulkanSwapChain = nullptr;
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

        // If the command buffer was already recorded once, then a call to vkBeginCommandBuffer() will implicitly reset it.
        VkResult errorCode = vkBeginCommandBuffer( commandBuffer, &beginInfo);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkBeginCommandBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

////////////// Starting a render pass
        // Drawing starts by beginning the render pass with vkCmdBeginRenderPass().
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanRenderPass;
        renderPassInfo.framebuffer = vulkanSwapchainFramebuffers[imageIndex];   // framebuffer for the swapchain image we want to draw to.

            // Specify size of the render area.
        renderPassInfo.renderArea.offset = { 0, 0};
        renderPassInfo.renderArea.extent = vulkanSwapChainExtent;

        // last two parameters define the clear values to use for 'VK_ATTACHMENT_LOAD_OP_CLEAR' which is used as
        // load operation for the color attachment.
        VkClearValue clearColor = {
            {   // VlClearColorValue
                { 0.0f, 0.0f, 0.0f, 1.0f}   // float32[4]
            }
        };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

            // begin render pass.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE /* hown drawing commands within the render pass will be provided. */);
        // VK_SUBPASS_CONTENTS_INLINE : The render pass commands will be embedded in the primary command buffer itself and no secondary buffers will be executed.
        // VK_SUBPASS_CONTENTS_SECONDARY_COMMNAD_BUFFERS : The render pass commands will be executed from secondary command buffers.


////////////// Drawing Commands
        // Bind Graphics Pipeline :- We now told Vulkan which operations to execute in the graphics pipeline and which attachment to use in the fragment shader.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline);

        // We already specify viewport and scissor state for this pipeline to be dyanmic. So we need to set them in command buffer
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanSwapChainExtent.width);
        viewport.height = static_cast<float>(vulkanSwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0};
        scissor.extent = vulkanSwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind Descriptor
        vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineLayout, 0, 1, &vulkanDescriptorSets[currentFrame], 0, nullptr);

        // Bind Vertex Buffer
        VkBuffer vertexBuffers[] = {vulkanVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Bind Index buffer
        vkCmdBindIndexBuffer(commandBuffer, vulkanIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Draw Command
        uint32_t instanceCount = sizeof(rectPushConstants) / sizeof(rectPushConstants[0]);
        for( uint32_t i = 0; i < instanceCount; ++i)
        {
            vkCmdPushConstants( commandBuffer, vulkanPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(struct PushConstant), &rectPushConstants[i]);
            vkCmdDrawIndexed( commandBuffer, /* indexCount */ static_cast<uint32_t>(indices.size()), /* instanceCount */ 1, /* firstIndex */ 0, /* vertexOffset */ 0, /* firstInstance */ 0);
        }

////////////// Finishing Up
        // The render pass can now be ended.
        vkCmdEndRenderPass(commandBuffer);

        // finish recording the command buffer
        errorCode = vkEndCommandBuffer(commandBuffer);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkEndCommandBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        return true;
    }

    /**
     * @brief UpdateUnifrmBuffer()
     */
    static void UpdateUnifrmBuffer(uint32_t currentImage)
    {
        // code
        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};

        ubo.modelMatrix = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.modelMatrix = glm::rotate(ubo.modelMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.viewMatrix = glm::lookAt(glm::vec3(.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        ubo.projectionMatrix = glm::perspective(glm::radians(45.0f), vulkanSwapChainExtent.width / (float)vulkanSwapChainExtent.height, 0.1f, 30.0f);

            // GLM was originally designed for OpenGL, where teh U coordinate of the clip coordinates is inverted. The easiest way to
            // compendate for that is to flip the sign on the scaling factor of the Y axis in the projection matrix.
        ubo.projectionMatrix[1][1] *= -1;

        // copy data to uniform buffer
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    /**
     * @brief DrawFrame()
     */
    void DrawFrame()
    {
    // Waiting for the previous frame.
        vkWaitForFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE /* Wait for all fences */, UINT64_MAX /* Timeout */);

    // Acquiring and image from the swap chain.
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR( vulkanLogicalDevice, vulkanSwapChain, UINT64_MAX /* timeout in nanoseconds for an image to become available (disable timeout here)*/, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Check if swap chain has become incompatible with the surface and can no longer be used for rendering. (Usually happen for resize).
        if(result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // recreate swap chain and try again in next DrawFrame() call.
            RecreateSwapChain();
            return;
        }
        else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LogError("Failed to acquire swap chain image!!!");
            return;
        }

    // Manually reset the fence to the unsignaled state.
        vkResetFences( vulkanLogicalDevice, 1, &inFlightFences[currentFrame]);

    // Recording the command buffer
        vkResetCommandBuffer( vulkanCommandBuffers[currentFrame], 0);  // Make sure it is able to be recorded.

        RecordCommandBuffer(vulkanCommandBuffers[currentFrame], imageIndex);

    // Update Uniforms
        UpdateUnifrmBuffer(currentFrame);

    //  Submitting the command buffer
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vulkanCommandBuffers[currentFrame];  // Command Buffers to actually submit for execution.

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;    // Specify which semaphores to wait on before execution begins.
        submitInfo.pWaitDstStageMask = waitStages;      // In which stage(s) of the pipeline to wait.
            // Each entry in the 'waitStage' array corresponds to the semaphore with the same index in 'pWaitSemaphores'
        
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;    // Specify which semaphores to signal once the command buffer(s) have finished execution.

        VkResult errorCode = vkQueueSubmit( vulkanGraphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkQueueSubmit() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return;
        }

// Presentation
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { vulkanSwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(vulkanPresentationQueue, &presentInfo);

        // Check if swap chain has become incompatible with the surface and can no longer be used for rendering. (Usually happen for resize).
        if(framebufferResized)
        {
            framebufferResized = false;

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
            if(vulkanUniformBuffers[i])
            {
                vkDestroyBuffer(vulkanLogicalDevice, vulkanUniformBuffers[i], nullptr);
                vulkanUniformBuffers[i] = nullptr;
            }

            if(vulkanUniformBuffersMemory[i])
            {
                vkUnmapMemory(vulkanLogicalDevice, vulkanUniformBuffersMemory[i]);
                uniformBuffersMapped[i] = nullptr;

                vkFreeMemory(vulkanLogicalDevice, vulkanUniformBuffersMemory[i], nullptr);
                vulkanUniformBuffersMemory[i] = nullptr;
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

        if(vulkanVertexBuffer)
        {
            vkDestroyBuffer(vulkanLogicalDevice, vulkanVertexBuffer, nullptr);
            vulkanVertexBuffer = nullptr;
        }

        if(vulkanVertexBufferMemory)
        {
            vkFreeMemory(vulkanLogicalDevice, vulkanVertexBufferMemory, nullptr);
            vulkanVertexBufferMemory = nullptr;
        }

        if(vulkanIndexBuffer)
        {
            vkDestroyBuffer(vulkanLogicalDevice, vulkanIndexBuffer, nullptr);
            vulkanIndexBuffer = nullptr;
        }

        if(vulkanIndexBufferMemory)
        {
            vkFreeMemory(vulkanLogicalDevice, vulkanIndexBufferMemory, nullptr);
            vulkanIndexBufferMemory = nullptr;
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

        if(vulkanGraphicsPipeline)
        {
            vkDestroyPipeline( vulkanLogicalDevice, vulkanGraphicsPipeline, nullptr);
            vulkanGraphicsPipeline = nullptr;
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

        if(enableValidationLayers && vulkanDebugMessenger)
        {
            DestroyDebugUtilsMessengerEXT(vulkanInstance, vulkanDebugMessenger, nullptr);
            vulkanDebugMessenger = nullptr;
        }

        if(vulkanSurface)
        {
            vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
            vulkanSurface = nullptr;
        }

        if(vulkanInstance)
        {
            vkDestroyInstance(vulkanInstance, nullptr);
            vulkanInstance = nullptr;
        }
    }
    
} // namespace VkApplication
