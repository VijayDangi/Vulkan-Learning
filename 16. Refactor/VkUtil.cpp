#include "VkUtil.h"

#include <vector>
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
        VkResult vulkanErrorCode = vkEnumerateInstanceExtensionProperties( nullptr, &vulkanExtensionCount, nullptr);
        if(vulkanErrorCode)
        {
            LogError("Vulkan: Failed to query instance extension properties count: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return;
        }

        // Get Instance Extension Properties.
        vulkanAvailableInstanceExtensionProperties.resize(vulkanExtensionCount);

        vulkanErrorCode = vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vulkanAvailableInstanceExtensionProperties.data());
        if(vulkanErrorCode)
        {
            LogError("Vulkan: Failed to query instance extensions properties : %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return;
        }

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
            VkResult vulkanErrorCode = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            if(vulkanErrorCode)
            {
                LogError("Vulkan: Failed to query instance layer properties count: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
                return;
            }

            vulkanAvailableInstanceLayerProperties.resize(layerCount);
            vulkanErrorCode = vkEnumerateInstanceLayerProperties(&layerCount, vulkanAvailableInstanceLayerProperties.data());
            if(vulkanErrorCode)
            {
                LogError("Vulkan: Failed to query instance layer properties: %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
                return;
            }

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

        vulkanErrorCode = vkCreateInstance(&createInfo, nullptr, pInstance);
        if(vulkanErrorCode != VK_SUCCESS)
        {
            LogError("vkCreateInstance() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(vulkanErrorCode));
            return false;
        }
        else
        {
            LogSuccess(enableValidationLayer ? "vkCreateInstance() Success with Validation Enabled." : "vkCreateInstance() Success with Validation Disabled.");
        }

        if(enableValidationLayer)
        {
            // setup validation layer debug callback.
            VkResult errorCode = __vkCreateDebugUtilsMessengerEXT(*pInstance, &debugMessengerCreateInfo, nullptr, &vulkanDebugMessenger);
            if(errorCode != VK_SUCCESS)
            {
                LogError("vkCreateDebugUtilsMessengerEXT() Failed: %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
                return false;
            }
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

        VkResult errorCode = vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfo, nullptr, pSurface);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkam: [Error] vkCreateWin32SurfaceKHR() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // LogSuccess("Vulkan: [Success] Vulkan Surface Create Successfully.");

        return true;
    }

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
        VkResult errorCode = vkCreateShaderModule( vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateShaderModule() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return VK_NULL_HANDLE;
        }

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
        VkResult errorCode = vkCreateShaderModule( vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateShaderModule() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return VK_NULL_HANDLE;
        }

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

        VkResult errorCode = vkCreateBuffer( vkDevice, &bufferInfo, nullptr, &buffer);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkCreateBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

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

        errorCode = vkAllocateMemory( vkDevice, &allocInfo, nullptr, &bufferMemory);
        if(errorCode != VK_SUCCESS)
        {
            LogError("Vulkan: [Error] vkAllocateBuffer() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

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

        VkResult errorCode = vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, out_commandPool);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkCreateCommandPool() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

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

        VkResult errorCode = vkAllocateCommandBuffers( vkDevice, &allocateInfo, pCommandBuffers);
        if(errorCode)
        {
            LogError("Vulkan: [Error] vkAllocateCommandBuffers() Failed. %s", VulkanHelper::GetVulkanErrorCodeString(errorCode));
            return false;
        }

        // LogSuccess("Vulkan: [Success] Create Command Buffer.");
        return true;
    }
};
