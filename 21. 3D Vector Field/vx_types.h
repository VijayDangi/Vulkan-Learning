#pragma once

#include "Common.h"
#include <vector>
#include <array>

    // Type Declaration
#define INVALID_QUEUE_FAMILY_HANDLE -1

namespace VX_FRAMEWORK_NAMESPACE
{ 
    namespace types
    {
        struct QueueFamilyIndices
        {
            uint32_t graphicsComputeFamily = INVALID_QUEUE_FAMILY_HANDLE;
            uint32_t presentFamily = INVALID_QUEUE_FAMILY_HANDLE;       // Queue-specific feature which is Presenting to the Surface.
            uint32_t transferFamily = INVALID_QUEUE_FAMILY_HANDLE;

            bool IsComplete()
            {
                // code
                return graphicsComputeFamily != INVALID_QUEUE_FAMILY_HANDLE &&
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

        struct VulkanImage
        {
            VkImage handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            VkImageCreateInfo imageInfo{};

            void Destroy(VkDevice device)
            {
                if(handle)
                {
                    vkDestroyImage(device, handle, nullptr);
                    handle = nullptr;
                }

                if(memory)
                {
                    vkFreeMemory(device, memory, nullptr);
                    memory = nullptr;
                }
            }
        };

        struct VulkanBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };

            void Destroy(VkDevice device)
            {
                if(handle)
                {
                    vkDestroyBuffer(device, handle, nullptr);
                    handle = nullptr;
                }

                if(memory)
                {
                    vkFreeMemory(device, memory, nullptr);
                    memory = nullptr;
                }
            }
        };

        struct VulkanUniformBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            void *mapped{nullptr};

            void Destroy(VkDevice device)
            {
                if(handle)
                {
                    vkDestroyBuffer(device, handle, nullptr);
                    handle = nullptr;
                }

                if(mapped)
                {
                    vkUnmapMemory(device, memory);
                    mapped = nullptr;
                }

                if(memory)
                {
                    vkFreeMemory(device, memory, nullptr);
                    memory = nullptr;
                }
            }
        };
    }
}
