#include "VkUtil.h"

#include <vector>
#include <set>
#include <string>
#include <fstream>

namespace VkUtil
{
    // variable declaration
    std::vector<VkLayerProperties> vulkanAvailableInstanceLayerProperties;
    std::vector<VkExtensionProperties> vulkanAvailableInstanceExtensionProperties;

    std::vector<const char*> vulkanAvailableInstanceLayerNames;
    std::vector<const char*> vulkanAvailableInstanceExtensionNames;
    VkDebugUtilsMessengerEXT vulkanDebugMessenger{ VK_NULL_HANDLE };

    static bool g_bEnableValidationLayer = false;


    // Function Definition
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

// ================================================================
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
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;

        bufferInfo.queueFamilyIndexCount = numQueueFamily;
        bufferInfo.pQueueFamilyIndices = queueFamilies;

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
};



// =======================================================//
// =======================================================//
// =======================================================//
namespace VkCustomAPI
{
    /**
    * @brief: LogVulkanPhysicalDevicesInfo()
    */
    void LogVulkanPhysicalDevicesInfo(VkInstance vkInstance)
   {
        // Listing the graphics cards.
        uint32_t deviceCount = 0;
        VkResult vulkanErrorCode = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        if(deviceCount == 0)
        {
            LogError("Vulkan [Error]: Failed to find GPUs with Vulkan Support!");
            return;
        }

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VK_UTIL_FN_ERROR_CHECK_RETURN(vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data()), );


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
   }

    /**
     * @brief LogSwapChainSupportDetails()
     */
    static void LogSwapChainSupportDetails(VkUtil::Types::SwapChainSupportDetails& details, const char *deviceName, VkPhysicalDeviceType deviceType)
    {
        // code
        std::string log_string;

        std::string deviceTypeName;
        switch(deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:             deviceTypeName = "OTHER";           break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:    deviceTypeName = "INTEGRATED GPU";  break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:      deviceTypeName = "DISCRETE GPU";    break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       deviceTypeName = "VIRTUAL GPU";     break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:               deviceTypeName = "CPU";             break;
            default:                                        deviceTypeName = "UNKNOWN";         break;
        }
        
        // Capabilities
        log_string = log_string + "Device Name: " + deviceName + "\n";
        log_string = log_string + "Device Type: " + deviceTypeName + "\n";
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
     * @brief GetQueueFamilies()
     */
    static VkUtil::Types::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        // code
        VkUtil::Types::QueueFamilyIndices queueIndices;

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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
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
     * @brief GetSwapChainSupportDetails()
     */
    VkUtil::Types::SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        // code
        VkUtil::Types::SwapChainSupportDetails details;

        // Query Surface Capabilities
        VK_UTIL_FN_ERROR_CHECK_RETURN( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &details.capabilities), VkUtil::Types::SwapChainSupportDetails());

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
        LogSwapChainSupportDetails(details, deviceProperties.deviceName, deviceProperties.deviceType);

        return details;
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
        VkUtil::Types::QueueFamilyIndices indices = GetQueueFamilies(device, surface);

        bool extensionSupported = CheckDeviceExtensionSupport(device, vulkanRequiredDeviceExtensions);

        bool swapChainAdequate = false;
        if(extensionSupported)
        {
            // For now Swap chain support if sufficient if there is at least one supported image format
            // and one supported presentation mode given the window surface we have.
            VkUtil::Types::SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.IsComplete() && extensionSupported && swapChainAdequate;
#endif
    }

    /**
     * @brief PickVulkanPhysicalDevice()
     */
    bool GetVulkanPhysicalDevice(VkInstance vkInstance, VkSurfaceKHR surface, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkPhysicalDevice *pPhysicalDevice, VkUtil::Types::QueueFamilyIndices *pQueueIndices)
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
    bool CreateVulkanLogicalDevice(VkPhysicalDevice physicalDevice, VkUtil::Types::QueueFamilyIndices& queueIndices, const std::vector<const char*>& vulkanRequiredDeviceExtensions, VkDevice *pDevice)
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
        std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsFamily, queueIndices.presentFamily, queueIndices.transferFamily};

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

        ///////////////// Specifying set of device features that we'll be using.
            // Right Now we don't need anythi special, so we can simply defint it and leave everyting to VK_FALSE.
        VkPhysicalDeviceFeatures deviceFeatures{};

        ///////////////// Creating the logical device
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());

        deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;

        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(vulkanRequiredDeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = vulkanRequiredDeviceExtensions.data();

        uint32_t layerCount = 0;
        const char** supportedInstanceLayers = VkUtil::GetVulkanInstanceSupportedLayers(&layerCount);
        deviceCreateInfo.enabledLayerCount       = layerCount;
        deviceCreateInfo.ppEnabledLayerNames     = supportedInstanceLayers;

        VK_UTIL_FN_ERROR_CHECK_RETURN(vkCreateDevice(physicalDevice /* Device to interface with */, &deviceCreateInfo, nullptr, pDevice), false);

        return true;
    }
}
