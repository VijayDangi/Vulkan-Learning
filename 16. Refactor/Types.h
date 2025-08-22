#pragma once

#include "Common.h"

    // Type Declaration
#define INVALID_QUEUE_FAMILY_HANDLE -1

namespace Vk
{
    namespace Types
    {
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

        struct VulkanBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
        };

        struct VulkanUniformBuffer
        {
            VkBuffer handle{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            void *mapped{nullptr};
        };
    }
}
