// @reference: Sascha Willems' Vulkan Examples

#define VK_USE_PLATFORM_WIN32_KHR   // Used to include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.h>

namespace VX_FRAMEWORK_NAMESPACE
{
    namespace initializers
    {
        /**
         * @brief memoryAllocateInfo() 
         */
        inline VkMemoryAllocateInfo memoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
        {
            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAllocInfo.allocationSize = allocationSize;
            memAllocInfo.memoryTypeIndex = memoryTypeIndex;
            return memAllocInfo;
        }

        /**
         * @brief mappedMemoryRange() 
         */
        inline VkMappedMemoryRange mappedMemoryRange(VkDeviceSize size, VkDeviceSize offset = 0)
        {
            VkMappedMemoryRange mappedMemoryRange{};
            mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedMemoryRange.size = size;
            mappedMemoryRange.offset = offset;
            return mappedMemoryRange;
        }

        /**
         * @brief commandPoolCreateInfo() 
         */
        inline VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex = 0, VkCommandPoolCreateFlags flags = 0)
        {
            VkCommandPoolCreateInfo cmdPoolCreateInfo{};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
            cmdPoolCreateInfo.flags = flags;
            return cmdPoolCreateInfo;
        }

        /**
         * @brief commandBufferAllocateInfo() 
         */
        inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
            cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocateInfo.commandPool = commandPool;
            cmdBufAllocateInfo.level = level;
            cmdBufAllocateInfo.commandBufferCount = commandBufferCount;
            return cmdBufAllocateInfo;
        }

        /**
         * @brief commandBufferBeginInfo() 
         */
        inline VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0)
        {
            VkCommandBufferBeginInfo cmdBufBeginInfo{};
            cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufBeginInfo.flags = flags;
            return cmdBufBeginInfo;
        }

        /**
         * @brief commandBufferInheritanceInfo() 
         */
        inline VkCommandBufferInheritanceInfo commandBufferInheritanceInfo()
        {
            VkCommandBufferInheritanceInfo cmdBufInheritanceInfo{};
            cmdBufInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            return cmdBufInheritanceInfo;
        }

        /**
         * @brief renderPassCreateInfo() 
         */
        inline VkRenderPassCreateInfo renderPassCreateInfo()
        {
            VkRenderPassCreateInfo renderPassCreateInfo{};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            return renderPassCreateInfo;
        }

        /**
         * @brief framebufferCreateInfo() 
         */
        inline VkRenderPassBeginInfo renderPassBeginInfo()
        {
            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            return renderPassBeginInfo;
        }

        inline VkImageMemoryBarrier imageMemoryBarrier()
        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return imageMemoryBarrier;
        }

        inline VkBufferMemoryBarrier bufferMemoryBarrier()
        {
            VkBufferMemoryBarrier bufferMemoryBarrier{};
            bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            return bufferMemoryBarrier;
        }

        inline VkMemoryBarrier memoryBarrier()
        {
            VkMemoryBarrier memoryBarrier{};
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            return memoryBarrier;
        }

        inline VkImageCreateInfo imageCreateInfo()
        {
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            return imageCreateInfo;
        }

        inline VkSamplerCreateInfo samplerCreateInfo()
        {
            VkSamplerCreateInfo samplerCreateInfo{};
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            return samplerCreateInfo;
        }

        inline VkImageViewCreateInfo imageViewCreateInfo()
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            return imageViewCreateInfo;
        }

        inline VkFramebufferCreateInfo framebufferCreateInfo()
        {
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            return framebufferCreateInfo;
        }

        inline VkSemaphoreCreateInfo semaphoreCreateInfo()
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            return semaphoreCreateInfo;
        }

        inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
        {
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = flags;
            return fenceCreateInfo;
        }

        inline VkSubmitInfo submitInfo()
        {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            return submitInfo;
        }

        inline VkViewport viewport( float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
        {
            VkViewport viewport{};
            viewport.x = x;
            viewport.y = y;
            viewport.width = width;
            viewport.height = height;
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;
            return viewport;
        }

        inline VkRect2D rect2D(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height)
        {
            VkRect2D rect2D{};
            rect2D.extent.width = width;
            rect2D.extent.height = height;
            rect2D.offset.x = offsetX;
            rect2D.offset.y = offsetY;
            return rect2D;
        }

        inline VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags usage, VkDeviceSize size, VkBufferCreateFlags flags = 0)
        {
            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.usage = usage;
            bufferCreateInfo.size = size;
            bufferCreateInfo.flags = flags;
            return bufferCreateInfo;
        }

        inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets, VkDescriptorPoolCreateFlags flags = 0)
        {
            VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
            descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolCreateInfo.poolSizeCount = poolSizeCount;
            descriptorPoolCreateInfo.pPoolSizes = pPoolSizes;
            descriptorPoolCreateInfo.maxSets = maxSets;
            descriptorPoolCreateInfo.flags = flags;
            return descriptorPoolCreateInfo;
        }

        inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
        {
            VkDescriptorPoolSize descriptorPoolSize{};
            descriptorPoolSize.type = type;
            descriptorPoolSize.descriptorCount = descriptorCount;
            return descriptorPoolSize;
        }

        inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1)
        {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
            descriptorSetLayoutBinding.binding = binding;
            descriptorSetLayoutBinding.descriptorType = descriptorType;
            descriptorSetLayoutBinding.descriptorCount = descriptorCount;
            descriptorSetLayoutBinding.stageFlags = stageFlags;
            return descriptorSetLayoutBinding;
        }

        inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(uint32_t bindingCount, VkDescriptorSetLayoutBinding* pBindings, VkDescriptorSetLayoutCreateFlags flags = 0)
        {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
            descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
            descriptorSetLayoutCreateInfo.pBindings = pBindings;
            descriptorSetLayoutCreateInfo.flags = flags;
            return descriptorSetLayoutCreateInfo;
        }

        inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, VkDescriptorSetLayout* pSetLayouts)
        {
            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = descriptorPool;
            descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
            descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
            return descriptorSetAllocateInfo;
        }

        inline VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
        {
            VkDescriptorImageInfo descriptorImageInfo{};
            descriptorImageInfo.sampler = sampler;
            descriptorImageInfo.imageView = imageView;
            descriptorImageInfo.imageLayout = imageLayout;
            return descriptorImageInfo;
        }

        inline VkDescriptorBufferInfo descriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
        {
            VkDescriptorBufferInfo descriptorBufferInfo{};
            descriptorBufferInfo.buffer = buffer;
            descriptorBufferInfo.offset = offset;
            descriptorBufferInfo.range = range;
            return descriptorBufferInfo;
        }

        inline VkWriteDescriptorSet writeBufferDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, VkDescriptorType descriptorType, VkDescriptorBufferInfo *bufferInfo, uint32_t descriptorCount = 1)
        {
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = dstSet;
            writeDescriptorSet.dstBinding = dstBinding;
            writeDescriptorSet.descriptorType = descriptorType;
            writeDescriptorSet.pBufferInfo = bufferInfo;
            writeDescriptorSet.descriptorCount = descriptorCount;
            return writeDescriptorSet;
        }

        inline VkWriteDescriptorSet writeImageDescriptorSet(VkDescriptorSet dstSet, uint32_t dstBinding, VkDescriptorType descriptorType, VkDescriptorImageInfo *imageInfo, uint32_t descriptorCount = 1)
        {
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = dstSet;
            writeDescriptorSet.dstBinding = dstBinding;
            writeDescriptorSet.descriptorType = descriptorType;
            writeDescriptorSet.pImageInfo = imageInfo;
            writeDescriptorSet.descriptorCount = descriptorCount;
            return writeDescriptorSet;
        }

        inline VkVertexInputBindingDescription vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
        {
            VkVertexInputBindingDescription vertexInputBindingDescription{};
            vertexInputBindingDescription.binding = binding;
            vertexInputBindingDescription.stride = stride;
            vertexInputBindingDescription.inputRate = inputRate;
            return vertexInputBindingDescription;
        }

        inline VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
        {
            VkVertexInputAttributeDescription vertexInputAttributeDescription{};
            vertexInputAttributeDescription.location = location;
            vertexInputAttributeDescription.binding = binding;
            vertexInputAttributeDescription.format = format;
            vertexInputAttributeDescription.offset = offset;
            return vertexInputAttributeDescription;
        }

        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
        {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            return pipelineVertexInputStateCreateInfo;
        }

        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo( uint32_t vertexBindingDescriptionCount, VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, VkVertexInputAttributeDescription* pVertexAttributeDescriptions, VkPipelineVertexInputStateCreateFlags flags = 0)
        {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexBindingDescriptionCount;
            pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = pVertexBindingDescriptions;
            pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
            pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = pVertexAttributeDescriptions;
            pipelineVertexInputStateCreateInfo.flags = flags;
            return pipelineVertexInputStateCreateInfo;
        }

        inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags = 0, VkBool32 primitiveRestartEnable = VK_FALSE)
        {
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = topology;
            pipelineInputAssemblyStateCreateInfo.flags = flags;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
            return pipelineInputAssemblyStateCreateInfo;
        }

        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(uint32_t setLayoutCount, VkDescriptorSetLayout* pSetLayouts, uint32_t pushConstantRangeCount = 0, VkPushConstantRange* pPushConstantRanges = nullptr, VkPipelineLayoutCreateFlags flags = 0)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
            pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
            pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRangeCount;
            pipelineLayoutCreateInfo.pPushConstantRanges = pPushConstantRanges;
            pipelineLayoutCreateInfo.flags = flags;
            return pipelineLayoutCreateInfo;
        }

        inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags = 0)
        {
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
            pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
            pipelineRasterizationStateCreateInfo.cullMode = cullMode;
            pipelineRasterizationStateCreateInfo.frontFace = frontFace;
            pipelineRasterizationStateCreateInfo.flags = flags;
            return pipelineRasterizationStateCreateInfo;
        }

        inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkBool32 blendEnable, VkColorComponentFlags colorWriteMask)
        {
            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
            pipelineColorBlendAttachmentState.blendEnable = blendEnable;
            pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
            return pipelineColorBlendAttachmentState;
        }

        inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(uint32_t attachmentCount, VkPipelineColorBlendAttachmentState* pAttachments, VkPipelineColorBlendStateCreateFlags flags = 0)
        {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
            pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
            pipelineColorBlendStateCreateInfo.flags = flags;
            return pipelineColorBlendStateCreateInfo;
        }

        inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkPipelineDepthStencilStateCreateFlags flags = 0)
        {
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
            pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
            pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
            pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
            pipelineDepthStencilStateCreateInfo.flags = flags;
            return pipelineDepthStencilStateCreateInfo;
        }

        inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(uint32_t viewportCount, VkViewport *pViewports, uint32_t scissorCount, VkRect2D *pScissors, VkPipelineViewportStateCreateFlags flags = 0)
        {
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
            pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipelineViewportStateCreateInfo.viewportCount = viewportCount;
            pipelineViewportStateCreateInfo.pViewports = pViewports;
            pipelineViewportStateCreateInfo.scissorCount = scissorCount;
            pipelineViewportStateCreateInfo.pScissors = pScissors;
            pipelineViewportStateCreateInfo.flags = flags;
            return pipelineViewportStateCreateInfo;
        }

        inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags = 0)
        {
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
            pipelineMultisampleStateCreateInfo.flags = flags;
            return pipelineMultisampleStateCreateInfo;
        }

        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(uint32_t dynamicStateCount, VkDynamicState* pDynamicStates, VkPipelineDynamicStateCreateFlags flags = 0)
        {
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
            pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
            pipelineDynamicStateCreateInfo.flags = flags;
            return pipelineDynamicStateCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass, VkPipelineCreateFlags flags = 0)
        {
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
            graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphicsPipelineCreateInfo.layout = layout;
            graphicsPipelineCreateInfo.renderPass = renderPass;
            graphicsPipelineCreateInfo.subpass = subpass;
            graphicsPipelineCreateInfo.flags = flags;
            graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            graphicsPipelineCreateInfo.basePipelineIndex = -1;
            return graphicsPipelineCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo()
        {
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
            graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            graphicsPipelineCreateInfo.basePipelineIndex = -1;
            return graphicsPipelineCreateInfo;
        }

        inline VkComputePipelineCreateInfo computePipelineCreateInfo(VkPipelineLayout layout, VkPipelineCreateFlags flags = 0)
        {
            VkComputePipelineCreateInfo computePipelineCreateInfo{};
            computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            computePipelineCreateInfo.layout = layout;
            computePipelineCreateInfo.flags = flags;
            computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            computePipelineCreateInfo.basePipelineIndex = -1;
            return computePipelineCreateInfo;
        }

        inline VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags;
            pushConstantRange.offset = offset;
            pushConstantRange.size = size;
            return pushConstantRange;
        }

        inline VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
        {
            VkSpecializationMapEntry specializationMapEntry{};
            specializationMapEntry.constantID = constantID;
            specializationMapEntry.offset = offset;
            specializationMapEntry.size = size;
            return specializationMapEntry;
        }

        inline VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, VkSpecializationMapEntry* pMapEntries, size_t dataSize, const void* pData)
        {
            VkSpecializationInfo specializationInfo{};
            specializationInfo.mapEntryCount = mapEntryCount;
            specializationInfo.pMapEntries = pMapEntries;
            specializationInfo.dataSize = dataSize;
            specializationInfo.pData = pData;
            return specializationInfo;
        }

        inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* pName, const VkSpecializationInfo* pSpecializationInfo = nullptr, VkPipelineShaderStageCreateFlags flags = 0)
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
            pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pipelineShaderStageCreateInfo.stage = stage;
            pipelineShaderStageCreateInfo.module = module;
            pipelineShaderStageCreateInfo.pName = pName;
            pipelineShaderStageCreateInfo.pSpecializationInfo = pSpecializationInfo;
            pipelineShaderStageCreateInfo.flags = flags;
            return pipelineShaderStageCreateInfo;
        }


        // =============== Ray Tracing Related Initializers ===============
        // @todo
    }
}
