#include "vx_utils.h"
#include "vx_initializers.hpp"

#include <vector>
#include <set>
#include <string>
#include <fstream>

#ifndef STBI_FAILURE_USERMSG
#define STBI_FAILURE_USERMSG
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace VX_FRAMEWORK_NAMESPACE
{
    namespace utils
    {
        // variable declaration
        std::vector<VkLayerProperties> vulkanAvailableInstanceLayerProperties;
        std::vector<VkExtensionProperties> vulkanAvailableInstanceExtensionProperties;

        std::vector<const char*> vulkanAvailableInstanceLayerNames;
        std::vector<const char*> vulkanAvailableInstanceExtensionNames;
        VkDebugUtilsMessengerEXT vulkanDebugMessenger{ VK_NULL_HANDLE };

        static bool g_bEnableValidationLayer = false;

        // Private Function Declaration
        static bool ReadFile(const std::string& filename, /* _Out_ */ std::vector<char>& out_data);
        static void GetVulkanSupportedInstanceExtensions();
        static void GetVulkanSupportedInstanceLayers();
        static int GetBytesPerTexFormat(VkFormat format);
        static VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& vulkanRequiredDeviceExtensions);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& vulkanRequiredDeviceExtensions);
        static VkResult __vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
        static void __vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);


        // ********************************************************************************************
        // Private Function Definition
        // ********************************************************************************************

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
         * @brief GetVulkanSupportedInstanceExtensions()
         *          Get Vulkan instance supported extensions.
         */
        static void GetVulkanSupportedInstanceExtensions()
        {
            // Get Instance Extension Properties Count.
            uint32_t vulkanExtensionCount = 0;
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateInstanceExtensionProperties( nullptr, &vulkanExtensionCount, nullptr),);

            // Get Instance Extension Properties.
            vulkanAvailableInstanceExtensionProperties.resize(vulkanExtensionCount);

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vulkanAvailableInstanceExtensionProperties.data()), );

            vulkanAvailableInstanceExtensionProperties.resize(vulkanExtensionCount);

            std::string out_string = "\n";
            out_string += "Vulkan Instances Extensions Properties: \n";
            for(uint32_t i = 0; i < vulkanExtensionCount; ++i)
            {
                out_string = out_string + "\t\tName: " + vulkanAvailableInstanceExtensionProperties[i].extensionName + "\n";
                out_string = out_string + "\t\tSpec Version: " +  std::to_string( vulkanAvailableInstanceExtensionProperties[i].specVersion) + " " + "\n\n";
                vulkanAvailableInstanceExtensionNames.push_back( vulkanAvailableInstanceExtensionProperties[i].extensionName);
            }

            Log("Vulkan Supported Extension Count: %u", vulkanExtensionCount);
            Log("%s", out_string.c_str());
        }

        /**
         * @brief GetVulkanSupportedInstanceLayers()
         *          Retrieve vulkan layer extension support.
         */
        static void GetVulkanSupportedInstanceLayers()
        {
            // code
            if(vulkanAvailableInstanceLayerProperties.size() == 0)
            {
                uint32_t layerCount = 0;
                VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateInstanceLayerProperties(&layerCount, nullptr), );

                vulkanAvailableInstanceLayerProperties.resize(layerCount);
                VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateInstanceLayerProperties(&layerCount, vulkanAvailableInstanceLayerProperties.data()), );

                std::string out_string = "\n";
                out_string += "Vulkan Instance Layer Properties: \n";
                for(uint32_t i = 0; i < layerCount; ++i)
                {
                    out_string = out_string + "\t\t" + "LayerName             : " + vulkanAvailableInstanceLayerProperties[i].layerName + "\n";
                    out_string = out_string + "\t\t" + "Description           : " + vulkanAvailableInstanceLayerProperties[i].description + "\n";
                    out_string = out_string + "\t\t" + "Implementation Version: " + std::to_string(vulkanAvailableInstanceLayerProperties[i].implementationVersion) + "\n";
                    out_string = out_string + "\t\t" + "Spec Version          : " + std::to_string( VK_API_VERSION_MAJOR(vulkanAvailableInstanceLayerProperties[i].specVersion)) + "."
                                                                                + std::to_string( VK_API_VERSION_MINOR(vulkanAvailableInstanceLayerProperties[i].specVersion)) + "."
                                                                                + std::to_string( VK_API_VERSION_PATCH(vulkanAvailableInstanceLayerProperties[i].specVersion))
                                                                                + "\n\n";

                    vulkanAvailableInstanceLayerNames.push_back( vulkanAvailableInstanceLayerProperties[i].layerName);
                }

                Log("Vulkan Supported Layer Count: %u", layerCount);
                Log("%s", out_string.c_str());
            }
        }

        /**
        * @name GetBytesPerTexFormat
        */
        static int GetBytesPerTexFormat(VkFormat format)
        {
            // code
            switch(format)
            {
                case VK_FORMAT_R8_SINT:
                case VK_FORMAT_R8_UNORM:
                    return 1;
                
                case VK_FORMAT_R16_SFLOAT:
                    return 2;
                
                case VK_FORMAT_R16G16_SFLOAT:
                case VK_FORMAT_R16G16_SNORM:
                case VK_FORMAT_B8G8R8A8_UNORM:
                case VK_FORMAT_R8G8B8A8_UNORM:
                case VK_FORMAT_R8G8B8A8_SNORM:
                case VK_FORMAT_R8G8B8A8_SRGB:
                    return 4;
                
                case VK_FORMAT_R16G16B16A16_SFLOAT:
                    return 4 * sizeof(uint16_t);
                
                case VK_FORMAT_R32G32B32A32_SFLOAT:
                    return 4 * sizeof(float);
                
                case VK_FORMAT_R8G8B8_SRGB:
                    return 3;

                case VK_FORMAT_R32G32B32_SFLOAT:
                    return 3 * sizeof(float);
                default:
                    LogError("Unknown format %d", format);
                    return 0;
            }
        }

        /**
         * @brief GetQueueFamilies()
         */
        static VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            // code
            VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices queueIndices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for(const VkQueueFamilyProperties& queueFamily : queueFamilies)
            {
                // Check if Device has Graphics queue, .
                if((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    queueIndices.graphicsComputeFamily = i;
                }

                if(!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    queueIndices.transferFamily = i;
                }

                // Check if device support surface presentation.
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if(presentSupport)
                {
                    queueIndices.presentFamily = i;
                }

                if(queueIndices.IsComplete())
                {
                    LogInfo("Graphics/Compute Queue Family Index: %u", queueIndices.graphicsComputeFamily);
                    LogInfo("Present Queue Family Index: %u", queueIndices.presentFamily);
                    LogInfo("Transfer Queue Family Index: %u", queueIndices.transferFamily);
                    break;
                }

                ++i;
            }

            return queueIndices;
        }

        /**
         * @brief IsPhysicalDeviceSuitable()
         *              Check whether device is suitable for the operations we want to perform.
         */
        static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& vulkanRequiredDeviceExtensions)
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
            VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices indices = GetQueueFamilies(device, surface);

            bool extensionSupported = CheckDeviceExtensionSupport(device, vulkanRequiredDeviceExtensions);

            bool swapChainAdequate = false;
            if(extensionSupported)
            {
                // For now Swap chain support if sufficient if there is at least one supported image format
                // and one supported presentation mode given the window surface we have.
                VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(device, surface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            return indices.IsComplete() && extensionSupported && swapChainAdequate;
    #endif
        }

        /**
         * @brief CheckDeviceExtensionSupport()
         */
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& vulkanRequiredDeviceExtensions)
        {
            // code
            uint32_t extensionCount = 0;
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr), false);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data()), false);

            std::set<std::string> requiredExtensions(vulkanRequiredDeviceExtensions.begin(), vulkanRequiredDeviceExtensions.end());

            for(const VkExtensionProperties& extension : availableExtensions)
            {
                // LogInfo("%s", extension.extensionName);
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        /**
         * @brief __vkCreateDebugUtilsMessengerEXT()
         */
        static VkResult __vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
        {
            // code
            // The above struct have to passed to vkCreateDebugUtilsMessagerEXT() function to create VkDebugUtilsMessengerEXT object.
            // Unfortunatly, because this function is an extension function, it is not automatically loaded.
            PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if(vkCreateDebugUtilsMessengerEXT)
            {
                return vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pDebugMessenger);
            }

            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        /**
         * @brief __vkDestroyDebugUtilsMessengerEXT()
         */
        static void __vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
        {
            // code
            // This function is an extension function, so it is needed to be explicitly loaded.
            PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if(vkDestroyDebugUtilsMessengerEXT)
            {
                vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, pAllocator);
            }
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
        VKAPI_ATTR VkBool32 VKAPI_CALL __VulkanDebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
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

        // ********************************************************************************************
        // Instance and Surface APIs
        // ********************************************************************************************

        /**
         * @brief FindVulkanMemoryType()
         */
        uint32_t FindMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            // code
            // Get available types of memory.
            VkPhysicalDeviceMemoryProperties memProperties{};
            vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

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
         * @brief CreateVulkanInstance() :
         *              Initialize the Vulkan Library by creating an instance.
         *              The instance is the connection between our application and the vulkan library and 
         *              creating it involves specifying some details about our application to the driver.
         */
        bool CreateVulkanInstance(const char * const *vulkanRequiredLayers, uint32_t numLayers, const char * const *vulkanRequiredExtensions, uint32_t numExtensions, bool enableValidationLayer, VkInstance* pInstance)
        {
            // code
            VkResult vulkanErrorCode;
            std::vector<const char*> requiredInstanceLayerNames;
            std::vector<const char*> requiredInstanceExtensionNames;

            // Vulkan Supported Extensions and Layers
            GetVulkanSupportedInstanceExtensions();
            GetVulkanSupportedInstanceLayers();

            for(uint32_t index = 0; index < numLayers; ++index)
            {
                requiredInstanceLayerNames.push_back(vulkanRequiredLayers[index]);
            }

            for(uint32_t index = 0; index < numExtensions; ++index)
            {
                requiredInstanceExtensionNames.push_back(vulkanRequiredExtensions[index]);
            }

            if(enableValidationLayer)
            {
                requiredInstanceLayerNames.push_back("VK_LAYER_KHRONOS_validation");
                requiredInstanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                g_bEnableValidationLayer = true;
            }
            else
            {
                for(std::vector<const char*>::iterator itr = requiredInstanceLayerNames.begin(); itr != requiredInstanceLayerNames.end(); )
                {
                    if(strcmp(*itr, "VK_LAYER_KHRONOS_validation") == 0)
                    {
                        itr = requiredInstanceLayerNames.erase(itr);
                    }
                    else
                    {
                        ++itr;
                    }
                }

                for(std::vector<const char*>::iterator itr = requiredInstanceExtensionNames.begin(); itr != requiredInstanceExtensionNames.end(); )
                {
                    if(strcmp(*itr, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                    {
                        itr = requiredInstanceExtensionNames.erase(itr);
                    }
                    else
                    {
                        ++itr;
                    }
                }

                g_bEnableValidationLayer = false;
            }

            // Check if specified extensions is supported by vulkan
            bool bFoundNotSupportedExtension = false;
            for(const char *extensionName : requiredInstanceExtensionNames)
            {
                bool extensionFound = false;

                for(const char*& supportedExtensionName : vulkanAvailableInstanceExtensionNames)
                {
                    if(strcmp(extensionName, supportedExtensionName) == 0)
                    {
                        extensionFound = true;
                        break;
                    }
                }

                if(!extensionFound)
                {
                    bFoundNotSupportedExtension = true;
                    Log("Extension: \"%s\" not support by vulkan", extensionName);
                }
            }

            if(bFoundNotSupportedExtension)
            {
                return false;
            }

            // Check if specified layers is supported by vulkan
            bool bFoundNotSupportedLayer = false;
            for(const char *layerName : requiredInstanceLayerNames)
            {
                bool layerFound = false;

                for(const char*& supportedLayerName : vulkanAvailableInstanceLayerNames)
                {
                    if(strcmp(layerName, supportedLayerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if(!layerFound)
                {
                    bFoundNotSupportedLayer = true;
                    Log("Layer: \"%s\" not support by vulkan", layerName);
                }
            }

            if(bFoundNotSupportedLayer)
            {
                return false;
            }

            ////////////////////////////////////
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
            appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName    = "Hello Triangle";
            appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName         = "No Engine";
            appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion          = VK_API_VERSION_1_0;
            appInfo.pNext               = nullptr;

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
            createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredInstanceExtensionNames.size());
            createInfo.ppEnabledExtensionNames = requiredInstanceExtensionNames.data();

            createInfo.enabledLayerCount = static_cast<uint32_t>(requiredInstanceLayerNames.size());
            createInfo.ppEnabledLayerNames = requiredInstanceLayerNames.data();
            
            VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
            if(enableValidationLayer)
            {
                // Enable Validation Layer.
                // To create a separate debug utils messenger specifically for 'vkCreateInstace()' and 'vkDestroyInstance()'
                // function calls. It requires you to simply pass a pointer to a 'vkDebugUtilsMessengerCreateInfoEXT' struct
                // in the 'pNext' extension field of 'vkInstanceCreateInfo()'.
                debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

                debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

                debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

                debugMessengerCreateInfo.pfnUserCallback = __VulkanDebugCallback;
                debugMessengerCreateInfo.pUserData = nullptr;
                
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
            }

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateInstance(&createInfo, nullptr, pInstance), false);
            LogSuccess(enableValidationLayer ? "vkCreateInstance() Success with Validation Enabled." : "vkCreateInstance() Success with Validation Disabled.");

            if(enableValidationLayer)
            {
                // setup validation layer debug callback.
                VK_UTIL_FN_ERROR_CHECK_RETURN( __vkCreateDebugUtilsMessengerEXT(*pInstance, &debugMessengerCreateInfo, nullptr, &vulkanDebugMessenger), false);
            }

            return true;
        }

        /**
         * @brief GetVulkanInstanceSupportedLayers()
         */
        const char** GetVulkanInstanceSupportedLayers(uint32_t *pNumLayers)
        {
            // code
            *pNumLayers = vulkanAvailableInstanceLayerNames.size();
            return vulkanAvailableInstanceLayerNames.data();
        }

        /**
         * @brief GetVulkanInstanceSupportedExtensions()
         */
        const char** GetVulkanInstanceSupportedExtensions(uint32_t *pNumExtensions)
        {
            // code
            *pNumExtensions = vulkanAvailableInstanceExtensionNames.size();
            return vulkanAvailableInstanceExtensionNames.data();
        }

        /**
         * @brief DestroyVulkanInstance()
         */
        void DestroyVulkanInstance(VkInstance *instance)
        {
            // code
            if(!instance && !(*instance))
            {
                LogError("Invalid VKInstane.");
                return;
            }

            if(vulkanDebugMessenger)
            {
                __vkDestroyDebugUtilsMessengerEXT(*instance, vulkanDebugMessenger, nullptr);
                vulkanDebugMessenger = nullptr;
            }

            vkDestroyInstance(*instance, nullptr);
            *instance = VK_NULL_HANDLE;
        }

        /**
         * @brief CreateVulkanSurface()
         */
        bool CreateVulkanSurface(VkInstance vkInstance, HWND windowHandle, VkSurfaceKHR *pSurface)
        {
            // code
            VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo{};
            win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            win32SurfaceCreateInfo.hwnd = windowHandle;
            win32SurfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfo, nullptr, pSurface), false);
            // LogSuccess("Vulkan: [Success] Vulkan Surface Create Successfully.");
            return true;
        }

        /**
         * @brief GetSwapChainSupportDetails()
         */
        VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            // code
            VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails details;

            // Query Surface Capabilities
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &details.capabilities), VX_FRAMEWORK_NAMESPACE::types::SwapChainSupportDetails());

            // Querying Supported surface formats.
            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if(formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            // Querying Present Modes
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, nullptr);
            if(presentModeCount != 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }


            VkPhysicalDeviceProperties deviceProperties{};
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            VX_FRAMEWORK_NAMESPACE::helper::LogSwapChainSupportDetails(details, deviceProperties.deviceName, deviceProperties.deviceType);

            return details;
        }
        
        
        /**
         * @brief PickVulkanPhysicalDevice()
         */
        bool GetVulkanPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR surface, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkPhysicalDevice *pPhysicalDevice, VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices *pQueueIndices)
        {
            // code
                // Listing the graphics cards.
            uint32_t deviceCount = 0;
            VkResult vulkanErrorCode = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
            if(deviceCount == 0)
            {
                LogError("Vulkan [Error]: Failed to find GPUs with Vulkan Support!");
                return false;
            }

            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data()), false);

            // Pick Physical Device
            *pPhysicalDevice = VK_NULL_HANDLE;

            for(const VkPhysicalDevice& device : physicalDevices)
            {
                if(IsPhysicalDeviceSuitable(device, surface, vulkanRequiredDeviceExtensions))
                {
                    VkPhysicalDeviceProperties deviceProperties{};
                    vkGetPhysicalDeviceProperties(device, &deviceProperties);

                    LogInfo("Vulkan: Selected Physical Device Name - \"%s\"", deviceProperties.deviceName);

                    *pPhysicalDevice = device;

                    // Get Queue Family Indices for selected device
                    *pQueueIndices = GetQueueFamilies(device, surface);
                    break;
                }
            }

            if((*pPhysicalDevice) == VK_NULL_HANDLE)
            {
                LogError("Vulkan [Error]: Failed to find suitable GPI!!!");
                return false;
            }

            return true;
        }

        /**
         * @brief CreateVulkanLogicalDevice()
         */
        bool CreateVulkanLogicalDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pDeviceFeatures, VX_FRAMEWORK_NAMESPACE::types::QueueFamilyIndices& queueIndices, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkDevice *pDevice)
        {
            // code
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
            // VkDeviceQueueCreateInfo::queueFamilyIndex must be unique
            std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsComputeFamily, queueIndices.presentFamily, queueIndices.transferFamily};

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            float queuePriority = 1.0f;

            for(uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;

                queueCreateInfos.push_back(queueCreateInfo);
            }
    #endif

            ///////////////// Creating the logical device
            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
            deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());

            deviceCreateInfo.pEnabledFeatures        = pDeviceFeatures;

            deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(vulkanRequiredDeviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = vulkanRequiredDeviceExtensions.data();

            uint32_t layerCount = 0;
            const char** supportedInstanceLayers = GetVulkanInstanceSupportedLayers(&layerCount);
            deviceCreateInfo.enabledLayerCount       = layerCount;
            deviceCreateInfo.ppEnabledLayerNames     = supportedInstanceLayers;

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDevice(physicalDevice /* Device to interface with */, &deviceCreateInfo, nullptr, pDevice), false);

            return true;
        }

        // =========================================
        /**
         * @brief CreateVkShaderModuleFromFile()
        */
        VkShaderModule CreateVkShaderModuleFromFile(VkDevice vkDevice, const std::string& filename)
        {
            // code
            std::vector<char> shader_code;
            if(!ReadFile(filename, shader_code))
            {
                return VK_NULL_HANDLE;
            }

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

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateShaderModule( vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule), VK_NULL_HANDLE);

            return shaderModule;
        }

        /**
         * @brief CreateVkShaderModuleFromSource()
        */
        VkShaderModule CreateVkShaderModuleFromSource(VkDevice vkDevice, const std::vector<char>& shader_code)
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

            VkShaderModule shaderModule = VK_NULL_HANDLE;
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateShaderModule( vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule), VK_NULL_HANDLE);

            return shaderModule;
        }

        /**
         * @brief CreateVkBuffer()
         */
        bool CreateVkBuffer(VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, uint32_t *queueFamilies, uint32_t numQueueFamily, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
        {
            // code
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;         // Sizeof of buffer.
            bufferInfo.usage = usage;       // Purpose of buffer (can specify multiple purposes).

            if(numQueueFamily == 0)
            {
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                bufferInfo.queueFamilyIndexCount = 0;
                bufferInfo.pQueueFamilyIndices = nullptr;
            }
            else
            {
                bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
                bufferInfo.queueFamilyIndexCount = numQueueFamily;
                bufferInfo.pQueueFamilyIndices = queueFamilies;
            }

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateBuffer( vkDevice, &bufferInfo, nullptr, &buffer), false);

            // Memory Requirement
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkDevice, buffer, &memRequirements);

            // Memory Allocation
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            uint32_t memoryType = FindMemoryType(vkPhysicalDevice, memRequirements.memoryTypeBits, memoryProperties);
            if(memoryType == -1)
            {
                LogError("FindMemoryType() Failed.");
                return false;
            }
            allocInfo.memoryTypeIndex = memoryType;

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkAllocateMemory( vkDevice, &allocInfo, nullptr, &bufferMemory), false);

            // Associate Memory with buffer
            vkBindBufferMemory( vkDevice, buffer, bufferMemory, 0);

            return true;
        }

        /**
         * @brief CopyBuffer()
         */
        void CopyVkBuffer(VkDevice vkDevice, VkCommandPool commandPool, VkQueue queue, VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
        {
            // code
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer);

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

            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

                // Unlike the draw commands, there are no events we need to wait on this time. We just want to execute
                // the transfer on the buffers immediately.
                // There are again two possible ways to wait on this transfer to complete. We could use a fence and wait with
                // 'vkWaitForFences()', or simply wait for the transfer queue to become idle with 'vkQueueWaitIdle()'.
            vkQueueWaitIdle(queue);

            vkFreeCommandBuffers(vkDevice, commandPool, 1, &commandBuffer);
            commandBuffer = nullptr;
        }

        /**
         * @brief CreateVkCommandPool()
         */
        bool CreateVkCommandPool(VkDevice vkDevice, VkCommandPoolCreateFlags poolCreateFlag, uint32_t queueFamilyIndex, VkCommandPool *out_commandPool)
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

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, out_commandPool), false);
            // LogSuccess("Vulkan: [Success] Vulkan Command Pool Created for QueueFamilyIndex: %u.", queueFamilyIndex);
            return true;
        }

        /**
         * @brief CreateVkCommandBuffers()
         */
        bool CreateVkCommandBuffers(VkDevice vkDevice, VkCommandPool commandPool, uint32_t bufferCount, VkCommandBuffer *pCommandBuffers)
        {
            // code
            VkCommandBufferAllocateInfo allocateInfo{};
            allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocateInfo.commandPool = commandPool;

            // VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
            // VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = bufferCount;

            VK_UTIL_FN_ERROR_CHECK_RETURN(vkAllocateCommandBuffers( vkDevice, &allocateInfo, pCommandBuffers), false);

            // LogSuccess("Vulkan: [Success] Create Command Buffer.");
            return true;
        }

        /**
         * @brief CreateVkFramebuffer()
        */
        bool CreateVkFramebuffer(VkDevice device, VkRenderPass renderPass, uint32_t attachment_count, VkImageView *pAttachments, uint32_t width, uint32_t height, uint32_t layers, VkFramebuffer *pFramebuffer)
        {
            // code
                // The attachments specified during render pass creation are bound by wrapping them into a 'VkFramebuffer' object.
                // A framebuffer object references all of the 'VkImageView' objects that represent the attachments.
            // Iterate through the image views and create framebuffers from them:
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.attachmentCount = attachment_count;
            framebufferCreateInfo.pAttachments = pAttachments;
            framebufferCreateInfo.width = width;
            framebufferCreateInfo.height = height;
            framebufferCreateInfo.layers = layers;

            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateFramebuffer( device, &framebufferCreateInfo, nullptr, pFramebuffer), false);

            return true;
        }

        // =====================================
        // Command Buffer will be get submitted to a queue and executed immediately. Also function will wait until the execution is finished before returning as the memory required to load the image date from file to ram must be freed once it is no longer needed.
        // Also command buffer will be allocated and freed inside the function as well.
        bool CreateTexture2D(const char *pFileName, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutDeviceMemory, bool flipTexture, bool generateMipMap)
        {
            // code
            VkCommandBuffer commandBuffer;
            if(!CreateVkCommandBuffers(device, commandPool, 1, &commandBuffer))
            {
                LogError("CreateVkCommandBuffers() Failed.");
                return false;
            }

            vkResetCommandBuffer(commandBuffer, 0);
            vkBeginCommandBuffer(commandBuffer, &VX_FRAMEWORK_NAMESPACE::initializers::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

            int imageWidth = 0;
            int imageHeight = 0;
            int imageChannels = 0;

            uint32_t mipLevels = 1;
            if(generateMipMap)
            {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageWidth, imageHeight)))) + 1;
            }

            // Load the image pixels
            stbi_set_flip_vertically_on_load(flipTexture ? 1 : 0);
            stbi_uc *pPixels = stbi_load(pFileName, &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);
            if(!pPixels)
            {
                LogError("Error Loading Texture from '%s'", pFileName);
                return false;
            }

            // Create a image object and populate it with pixels
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
            if(CreateImage(device, physicalDevice, imageWidth, imageHeight, mipLevels, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT, false, pOutImageInfo, pOutImage, pOutDeviceMemory))
            {
                LogError("CreateImage() Failed.");
                
                stbi_image_free(pPixels);
                pPixels = nullptr;

                vkEndCommandBuffer(commandBuffer);
                vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
                return false;
            }

            // Copy the image data from CPU to GPU
            if(CopyTextureDataToImage(device, physicalDevice, commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, *pOutImageInfo, *pOutImage, false, pPixels))
            {
                LogError("CopyTextureDataToImage() Failed.");
                
                stbi_image_free(pPixels);
                pPixels = nullptr;

                vkEndCommandBuffer(commandBuffer);
                vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
                return false;
            }

            //     TransitionImageLayout(*pOutImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount, 1, textureMipLevels - 1);

            if(mipLevels > 1)
            {
                ImageMemoryBarrier(commandBuffer, *pOutImage, (*pOutImageInfo).format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1, mipLevels - 1);
                GenerateMipmaps( device, commandBuffer, *pOutImage, (*pOutImageInfo).format, VK_IMAGE_ASPECT_COLOR_BIT, imageWidth, imageHeight, mipLevels, 1U);
            }


            // Submit Command Buffer and wait for it to finish
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = VX_FRAMEWORK_NAMESPACE::initializers::submitInfo();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);

            // Cleanup
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

            // Release the image pixels
            stbi_image_free(pPixels);
            pPixels = nullptr;

            LogSuccess("Texture '%s' created.", pFileName);
            return true;
        }

        bool CreateTextureCubeMapFromEquirectangular(const char *pFileName, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutDeviceMemory, bool flipTexture, bool generateMipMap)
        {
            // code
            VkCommandBuffer commandBuffer;
            if(!CreateVkCommandBuffers(device, commandPool, 1, &commandBuffer))
            {
                LogError("CreateVkCommandBuffers() Failed.");
                return false;
            }

            vkResetCommandBuffer(commandBuffer, 0);
            vkBeginCommandBuffer(commandBuffer, &VX_FRAMEWORK_NAMESPACE::initializers::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));

            int imageWidth = 0;
            int imageHeight = 0;
            int imageChannels = 0;

            uint32_t mipLevels = 1;
            if(generateMipMap)
            {
                mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageWidth, imageHeight)))) + 1;
            }

            // Load the image pixels
            stbi_set_flip_vertically_on_load(flipTexture ? 1 : 0);
            const stbi_uc *pPixels = stbi_load(pFileName, &imageWidth, &imageHeight, nullptr, STBI_rgb_alpha);
            if(!pPixels)
            {
                LogError("Error Loading Texture from '%s'", pFileName);
                return false;
            }
            LogInfo("\n FileName: %s\n Width: %d\n Height: %d\n Channels: %d\n", pFileName, imageWidth, imageHeight, imageChannels);

            std::vector<std::vector<unsigned char>> cubemapFaceData;
            uint32_t faceSize = ConvertEquirectangularImageToCubemap( pPixels, imageWidth, imageHeight, cubemapFaceData);

            stbi_image_free((void*)pPixels);
            pPixels = nullptr;

            // Hack...
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
            uint32_t bytesPerPixels = GetBytesPerTexFormat(format);
            size_t singleFaceNumBytes = faceSize * faceSize * bytesPerPixels;
            size_t totalBytes = CUBEMAP_NUM_FACE * singleFaceNumBytes;

            char *p = (char*) malloc(totalBytes);
            for(uint32_t i = 0; i < CUBEMAP_NUM_FACE; ++i)
            {
                memcpy( p + i * singleFaceNumBytes, cubemapFaceData[i].data(), singleFaceNumBytes);
            }

            // Create a image object and populate it with pixels
            if(CreateImage(device, physicalDevice, faceSize, faceSize, mipLevels, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT, true, pOutImageInfo, pOutImage, pOutDeviceMemory))
            {
                LogError("CreateImage() Failed.");

                if(p) { free(p); p = nullptr;}

                vkEndCommandBuffer(commandBuffer);
                vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
                return false;
            }

            // Copy the image data from CPU to GPU
            if(CopyTextureDataToImage(device, physicalDevice, commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, *pOutImageInfo, *pOutImage, true, p))
            {
                LogError("CopyTextureDataToImage() Failed.");

                if(p) { free(p); p = nullptr;}

                vkEndCommandBuffer(commandBuffer);
                vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
                return false;
            }

            if(mipLevels > 1)
            {
                ImageMemoryBarrier(commandBuffer, *pOutImage, (*pOutImageInfo).format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1, mipLevels - 1);
                GenerateMipmaps( device, commandBuffer, *pOutImage, (*pOutImageInfo).format, VK_IMAGE_ASPECT_COLOR_BIT, imageWidth, imageHeight, mipLevels, 6U);
            }

            // Submit Command Buffer and wait for it to finish
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo = VX_FRAMEWORK_NAMESPACE::initializers::submitInfo();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(queue);

            // Cleanup
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

            if(p) { free(p); p = nullptr;}

            LogSuccess("Texture '%s' created.", pFileName);
            return true;
        }

        /**
         * @name VulkanWrapper::ImageMemoryBarrier
         */
        bool ImageMemoryBarrier(VkCommandBuffer cmdBuf, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layoutCount, uint32_t mipBaseLevel, uint32_t mipLevelCount)
        {
            // code
            VkImageMemoryBarrier barrier = VX_FRAMEWORK_NAMESPACE::initializers::imageMemoryBarrier();
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspectMask;
            barrier.subresourceRange.baseMipLevel = mipBaseLevel;
            barrier.subresourceRange.levelCount = mipLevelCount;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = layoutCount;

            VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
            VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

            // ======
            if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
                /* Convert back from read-only to updateable */
            else if(oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
                /* Convert from updateable texture to shader read-only */
            else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
                /* Convert depth texture from undefined state to depth-stencil buffer */
            else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
                /* Wait for render pass to complete */
            else if(oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = 0;

                sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
                /* Convert back from read-only to color attachment */
            else if(oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }
                /* Convert from updateable texture to shader read-only */
            else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
                /* Convert back from read-only to depth attachment */
            else if(oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }
                /* Convert from updateable depth texture to shader read-only */
            else if(oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
                /* Transition from undefined layout to color attachment */
            else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            }
                /* Transition from color attachment to prensetation layout */
            else if(oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = 0;

                sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }

            vkCmdPipelineBarrier(cmdBuf, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            return true;
        }

        // void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, uint32_t mipBaseLevel, uint32_t mipLevelCount)
        // {
        //     // code
        //     vkResetCommandBuffer(m_CopyCommandBuffer, 0);
        //     VulkanWrapper::BeginCommandBuffer(m_CopyCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        //     VulkanWrapper::ImageMemoryBarrier(m_CopyCommandBuffer, image, format, oldLayout, newLayout, layerCount, mipBaseLevel, mipLevelCount);
        //     SubmitCopyCommand();
        // }

        // Should add transfer src and dst usage flags when creating the image if generating mipmap. Because we need to copy data from staging buffer to image and also copy data from one mip level to another mip level.
        bool CreateImage(
            VkDevice device, VkPhysicalDevice physicalDevice,
            int imageWidth, int imageHeight, uint32_t mipLevels,
            VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlagBits propertyFlags,
            VkSampleCountFlagBits numSamples,
            bool isCubemap,
            VkImageCreateInfo *pOutImageInfo, VkImage *pOutImage, VkDeviceMemory *pOutMemory)
        {
            // code
            // Calculate mip-map level
            uint32_t textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageWidth, imageHeight)))) + 1;
            mipLevels = std::max(std::min(textureMipLevels, mipLevels), 1u);

            // usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            VkImageCreateInfo imageInfo;
            memset(&imageInfo, 0, sizeof(imageInfo));

            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.pNext = nullptr;
            imageInfo.flags = isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = format;

            imageInfo.extent.width = imageWidth;
            imageInfo.extent.height = imageHeight;
            imageInfo.extent.depth = 1;

            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = isCubemap ? 6u : 1u;
            imageInfo.samples = numSamples;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = usage;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.queueFamilyIndexCount = 0;
            imageInfo.pQueueFamilyIndices = nullptr;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            // create the image object
            VkResult vk_fn_result = vkCreateImage(device, &imageInfo, nullptr, pOutImage);
            if(vk_fn_result != VK_SUCCESS)
            {
                LogError("vkCreateImage() Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(vk_fn_result));
                return false;
            }

            // get the image memory requirements
            VkMemoryRequirements memReqs;
            memset(&memReqs, 0, sizeof(memReqs));
            vkGetImageMemoryRequirements(device, *pOutImage, &memReqs);
            LogDebug("Image requires %d bytes", (int)memReqs.size);

            // get the memory type index
            uint32_t memoryTypeIndex = FindMemoryType( physicalDevice, memReqs.memoryTypeBits, propertyFlags);
            LogDebug("Image type index %d", memoryTypeIndex);

            // allocate memory
            VkMemoryAllocateInfo memAllocInfo;
            memset(&memAllocInfo, 0, sizeof(memAllocInfo));

            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.pNext = nullptr;
            memAllocInfo.allocationSize = memReqs.size;
            memAllocInfo.memoryTypeIndex = memoryTypeIndex;

            vk_fn_result = vkAllocateMemory(device, &memAllocInfo, nullptr, pOutMemory);
            if(vk_fn_result != VK_SUCCESS)
            {
                LogError("vkAllocateMemory() Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(vk_fn_result));
                return false;
            }

            // bind memory
            vk_fn_result = vkBindImageMemory(device, *pOutImage, *pOutMemory, 0);
            if(vk_fn_result != VK_SUCCESS)
            {
                LogError("vkBindImageMemory() Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(vk_fn_result));
                return false;
            }

            *pOutImageInfo = imageInfo;

            return true;
        }

        bool CopyBufferToImage(VkCommandBuffer commandBuffer, VkImage image, VkBuffer buffer, int imageWidth, int imageHeight, uint32_t layerCount, uint32_t mipLevel)
        {
            // code
            VkBufferImageCopy bufferImageCopy;
            bufferImageCopy.bufferOffset = 0;
            bufferImageCopy.bufferRowLength = 0;
            bufferImageCopy.bufferImageHeight = 0;

            bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferImageCopy.imageSubresource.mipLevel = mipLevel;
            bufferImageCopy.imageSubresource.baseArrayLayer = 0;
            bufferImageCopy.imageSubresource.layerCount = layerCount;

            bufferImageCopy.imageOffset.x = 0;
            bufferImageCopy.imageOffset.y = 0;
            bufferImageCopy.imageOffset.z = 0;

            bufferImageCopy.imageExtent.width = imageWidth;
            bufferImageCopy.imageExtent.height = imageHeight;
            bufferImageCopy.imageExtent.depth = 1;

            vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
            return true;
        }

        // Image Level 0 will be in transfer dst layout after this function. If not generating mipmap then it should be required layout.
        bool CopyTextureDataToImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, VkImageAspectFlags aspectFlags, VkImageCreateInfo& imageInfo, VkImage& image, bool isCubemap, const void *pPixels)
        {
            // code
            if(!pPixels || imageInfo.extent.width == 0 || imageInfo.extent.height == 0 || image == VK_NULL_HANDLE)
            {
                LogError("Invalid arguments.");
                return false;
            }

            // Copying data to image
            uint32_t layerCount = isCubemap ? 6u : 1u;
            int bytesPerPixel = GetBytesPerTexFormat(imageInfo.format);
            if(bytesPerPixel == 0)
            {
                return false;
            }

            VkDeviceSize layerSize = imageInfo.extent.width * imageInfo.extent.height * bytesPerPixel;
            VkDeviceSize imageSize = layerCount * layerSize;

            VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            VX_FRAMEWORK_NAMESPACE::types::VulkanBuffer stagingBuffer;
            memset(&stagingBuffer, 0, sizeof(stagingBuffer));

            if(!CreateVkBuffer(physicalDevice, device, imageSize, usage, properties, nullptr, 0, stagingBuffer.handle, stagingBuffer.memory))
            {
                LogError("CreateBuffer() Failed.");
                return false;
            }

            // Copy Data To Buffer
            void *mappedMemAdd = nullptr;
            VkResult vk_fn_result = vkMapMemory(device, stagingBuffer.memory, 0, imageSize, 0, &mappedMemAdd);
            if(vk_fn_result != VK_SUCCESS)
            {
                LogError("vkMapMemory() Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(vk_fn_result));

                stagingBuffer.Destroy(device);

                return false;
            }
            memcpy(mappedMemAdd, pPixels, imageSize);
            vkUnmapMemory(device, stagingBuffer.memory);
            mappedMemAdd = nullptr;

                // Transition first layer(0) from undefined to transfer dst.
            ImageMemoryBarrier(commandBuffer, image, imageInfo.format, aspectFlags, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount, 0, 1);

                // Copy buffer content to first layer(0) of image
            if(!CopyBufferToImage(commandBuffer, image, stagingBuffer.handle, imageInfo.extent.width, imageInfo.extent.height, layerCount, 0))
            {
                LogError("CopyBufferToImage() Failed.");
                stagingBuffer.Destroy(device);
                return false;
            }

            //     // If not generating mipmaps then transition layer to shader read only
            // if(!(generateMipMap && pPixels))
            // {
            //     TransitionImageLayout(*pOutImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerCount);
            // }

            // Clean up
            stagingBuffer.Destroy(device);

            // Generate Mipmap
            // if(generateMipMap && pPixels)
            // {
            //         // Before generating mipmap, make transition of each layer (starting from 1 to mipLevels) to transfer dst. [Not doing for layer 0 as it is already in transfer dst.]
            //     TransitionImageLayout(*pOutImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount, 1, textureMipLevels - 1);

            //         // Generate Mipmaps
            //     GenerateMipmaps(*pOutImage, outImageInfo.format, imageWidth, imageHeight, textureMipLevels, isCubemap ? 6U : 1U);
            // }
            return true;
        }

        // It is expected that the image layout of all mip levels is 'VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL' before calling this function. After this function, all mip levels should be transition to required layout.
        bool GenerateMipmaps( VkDevice device, VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layerCount)
        {
            // code
            {
                    // Not using TransitionImageLayout() as here we do all work in single command buffer and will submit in single queue call.
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.image = image;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.subresourceRange.aspectMask = aspectFlags;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = layerCount;

                int32_t mipWidth = width;
                int32_t mipHeight = height;

                for(uint32_t i = 1; i < mipLevels; ++i)
                {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                    vkCmdPipelineBarrier(
                        commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier
                    );

                    VkImageBlit blit{};
                        // Souce
                    blit.srcOffsets[0] = { 0, 0, 0};
                    blit.srcOffsets[1] = { mipWidth, mipHeight, 1};
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = layerCount;
                        // Destination
                    blit.dstOffsets[0] = { 0, 0, 0};
                    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = i;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = layerCount;

                    vkCmdBlitImage(commandBuffer,
                        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &blit,
                        VK_FILTER_LINEAR
                    );

                    // barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    // barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    // vkCmdPipelineBarrier(
                    //     commandBuffer,
                    //     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    //     0, nullptr,
                    //     0, nullptr,
                    //     1, &barrier
                    // );

                    if(mipWidth > 1)
                    {
                        mipWidth /= 2;
                    }

                    if(mipHeight > 1)
                    {
                        mipHeight /= 2;
                    }
                }

                // This Barrier transitions the last mip level from 'VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL' to 'VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL'.
                // This wasn't handled by the loop, since the last mip level is never blitted from.
                // barrier.subresourceRange.baseMipLevel = mipLevels - 1;
                // barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                // barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                // vkCmdPipelineBarrier(
                //     commandBuffer,
                //     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                //     0, nullptr,
                //     0, nullptr,
                //     1, &barrier
                // );
            }

            return true;
        }

        bool CreateVkImageViews(VkDevice device, VkImage image, VkImageViewType viewtype, VkFormat format, VkComponentMapping componentMapping, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkImageView& imageView)
        {
            // code
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = image;

                // Specify how the image data should be interpreted.
            imageViewCreateInfo.viewType = viewtype;
            imageViewCreateInfo.format = format;

                // Component Mapping
            imageViewCreateInfo.components.r = componentMapping.r;
            imageViewCreateInfo.components.g = componentMapping.g;
            imageViewCreateInfo.components.b = componentMapping.b;
            imageViewCreateInfo.components.a = componentMapping.a;

                // The 'subresourceRange' field describes what the image's purpose is and which part of the image should be accessed.
                // Our images will be used as color targets without any mipmapping levels or multiple layers.
            imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
            imageViewCreateInfo.subresourceRange.baseMipLevel = baseMipLevel;
            imageViewCreateInfo.subresourceRange.levelCount = levelCount;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
            imageViewCreateInfo.subresourceRange.layerCount = layerCount;

            // Create Image View.
            VK_UTIL_FN_ERROR_CHECK_RETURN( vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), false);

            return true;
        }
    }
}
