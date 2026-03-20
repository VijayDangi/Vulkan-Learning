
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR   // Used to include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../Common/glm/glm.hpp"
#include "../Common/glm/gtc/matrix_transform.hpp"

#include <array>
#include <vector>

namespace vk_types
{
    struct SVertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texcoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            // code
            // SVertex binding describes at which rate to load data from memory throughout the vertices.
            // It specifies the number of bytes between data entries and whether to move to the next data after
            // each vertex or after each instance.
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;                                 // Specifies the index of the binding in the array of bindings.
            bindingDescription.stride = sizeof(SVertex);                     // Specifies the number of bytes from one entry to the next.
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;     // PerVertex Or PerInstance.

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            // code
            std::array<VkVertexInputAttributeDescription, 3> attributeDescription{};

            // position
            attributeDescription[0].binding = 0;
            attributeDescription[0].location = 0;   // Shader Input Location Attribute.
            attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescription[0].offset = offsetof(SVertex, position);

            // color
            attributeDescription[1].binding = 0;
            attributeDescription[1].location = 1;   // Shader Input Location Attribute.
            attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescription[1].offset = offsetof(SVertex, color);

            // texcoord
            attributeDescription[2].binding = 0;
            attributeDescription[2].location = 2;   // Shader Input Location Attribute.
            attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescription[2].offset = offsetof(SVertex, texcoord);

            return attributeDescription;
        }
    
        bool operator==(const SVertex& other) const
        {
            return position == other.position &&
                   color == other.color &&
                   texcoord == other.texcoord;
        }
    };

    struct SVulkanBuffer
    {
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;
        size_t m_DataSize;

        SVulkanBuffer()
        {
            m_Buffer = VK_NULL_HANDLE;
            m_Memory = VK_NULL_HANDLE;
            m_DataSize = 0;
        }

        void Destroy(VkDevice device)
        {
            if(m_Buffer)
            {
                vkDestroyBuffer((device), m_Buffer, nullptr);
                m_Buffer = VK_NULL_HANDLE;
            }
            if(m_Memory)
            {
                vkFreeMemory((device), m_Memory, nullptr);
                m_Memory = VK_NULL_HANDLE;
            }
            m_DataSize = 0;
        }
    };

    struct SVulkanUniformBuffer
    {
        VkBuffer m_Buffer;
        VkDeviceMemory m_Memory;
        void *m_Mapped;
        size_t m_DataSize;

        SVulkanUniformBuffer()
        {
            m_Buffer = VK_NULL_HANDLE;
            m_Memory = VK_NULL_HANDLE;
            m_Mapped = nullptr;
            m_DataSize = 0;
        }

        void Destroy(VkDevice device)
        {
            if(m_Mapped)
            {
                vkUnmapMemory(device, m_Memory);
                m_Mapped = nullptr;
            }
            if(m_Buffer)
            {
                vkDestroyBuffer((device), m_Buffer, nullptr);
                m_Buffer = VK_NULL_HANDLE;
            }
            if(m_Memory)
            {
                vkFreeMemory((device), m_Memory, nullptr);
                m_Memory = VK_NULL_HANDLE;
            }
            m_DataSize = 0;
        }
    };

    struct SVulkanTexture
    {
        VkImage m_Image;
        VkDeviceMemory m_Memory;
        VkImageView m_View;

        SVulkanTexture()
        {
            m_Image = VK_NULL_HANDLE;
            m_Memory = VK_NULL_HANDLE;
            m_View = VK_NULL_HANDLE;
        }

        void Destroy(VkDevice device)
        {
            if(m_View)
            {
                vkDestroyImageView((device), m_View, nullptr);
                m_View = VK_NULL_HANDLE;
            }
            if(m_Image)
            {
                vkDestroyImage((device), m_Image, nullptr);
                m_Image = VK_NULL_HANDLE;
            }
            if(m_Memory)
            {
                vkFreeMemory((device), m_Memory, nullptr);
                m_Memory = VK_NULL_HANDLE;
            }
        }
    };

    // Define a structure to hold per-object data
    struct SGameObject
    {
        // Transform Properties
        glm::vec3 position = { 0.f, 0.f, 0.f };
        glm::vec3 rotation = { 0.f, 0.f, 0.f };
        glm::vec3 scale = { 1.f, 1.f, 1.f };

        // Uniform Buffer
        std::vector<SVulkanUniformBuffer> uniformBuffers{};

        // Descriptor sets for this object (one per frame in flight)
        std::vector<VkDescriptorSet> descriptorSets;

        glm::mat4 getModelMatrix() const {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale);
            return model;
        }
    };

    #define INVALID_QUEUE_FAMILY_HANDLE -1
    struct SQueueFamilyIndices
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

    struct SSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
}
