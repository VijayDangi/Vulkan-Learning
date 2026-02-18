#include <vulkan/vulkan.h>
#include "VulkanHelper.h"

namespace VulkanHelper
{
    /**
     * @brief GetVulkanErrorCodeString()
     *          Get Error string of specified vulkan error code.
     */
    const char* GetVulkanErrorCodeString(VkResult errorCode)
    {
        switch(errorCode)
        {
            case VK_SUCCESS:                                                return "Success. (VK_SUCCESS)";
            case VK_NOT_READY:                                              return "A Fence of Query has not yet completed. (VK_NOT_READY)";
            case VK_TIMEOUT:                                                return "A wait operation has not completed in the specified time. (VK_TIMEOUT)";
            case VK_EVENT_SET:                                              return "An event is signaled. (VK_EVENT_SET)";
            case VK_EVENT_RESET:                                            return "An evenet is unsignaled. (VK_EVENT_RESET)";
            case VK_INCOMPLETE:                                             return "A return array was too small for the result. (VK_INCOMPLETE)";
            case VK_ERROR_OUT_OF_HOST_MEMORY:                               return "A host memory allocation has failed. (VK_ERROR_OUT_OF_HOST_MEMORY)";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:                             return "A device memory allocation has failed. (VK_ERROR_OUT_OF_DEVICE_MEMORY)";
            case VK_ERROR_INITIALIZATION_FAILED:                            return "Initialization of an object could not be completed for implementation-specific reasons. (VK_ERROR_INITIALIZATION_FAILED)";
            case VK_ERROR_DEVICE_LOST:                                      return "The logical or physical device has been lost. (VK_ERROR_DEVICE_LOST)";
            case VK_ERROR_MEMORY_MAP_FAILED:                                return "Mapping of a memory object has failed. (VK_ERROR_MEMORY_MAP_FAILED)";
            case VK_ERROR_LAYER_NOT_PRESENT:                                return "A requested layer is not present or could not be loaded. (VK_ERROR_LAYER_NOT_PRESENT)";
            case VK_ERROR_EXTENSION_NOT_PRESENT:                            return "A requested extension is not supported. (VK_ERROR_EXTENSION_NOT_PRESENT)";
            case VK_ERROR_FEATURE_NOT_PRESENT:                              return "A requested feature is not supported. (VK_ERROR_FEATURE_NOT_PRESENT)";
            case VK_ERROR_INCOMPATIBLE_DRIVER:                              return "The requested version of vulkan is not supported by the driver or is otherwise incompatible. (VK_ERROR_INCOMPATIBLE_DRIVER)";
            case VK_ERROR_TOO_MANY_OBJECTS:                                 return "Too many objects of the type have already been created. (VK_ERROR_TOO_MANY_OBJECTS)";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:                             return "A requested format is not supported on this device. (VK_ERROR_FORMAT_NOT_SUPPORTED)";
            case VK_ERROR_FRAGMENTED_POOL:                                  return "A pool allocation has failed due to fragmentation of the pool's memory. (VK_ERROR_FRAGMENTED_POOL)";
            case VK_ERROR_UNKNOWN:                                          return "An Unknown error. (VK_ERROR_UNKNOWN)";
            case VK_ERROR_OUT_OF_POOL_MEMORY:                               return "A pool memory allocation has failed. (VK_ERROR_OUT_OF_POOL_MEMORY)";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:                          return "An external handle is not a valid handle of the specified type. (VK_ERROR_INVALID_EXTERNAL_HANDLE)";
            case VK_ERROR_FRAGMENTATION:                                    return "A descriptor pool creation has failed due to fragmentation. (VK_ERROR_FRAGMENTATION)";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:                   return "A buffer creation or memory allocation failed because the requested address is not available. (VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)";
            case VK_PIPELINE_COMPILE_REQUIRED:                              return "A requested pipeline creation would have required compilation, but the application requested compilation to not be performed. (VK_PIPELINE_COMPILE_REQUIRED)";
            case VK_ERROR_SURFACE_LOST_KHR:                                 return "A surface is no longer available. (VK_ERROR_SURFACE_LOST_KHR)";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                         return "The requested window is already in use by Vulkan or another API in a manner which prevents if from being used again. (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)";
            case VK_SUBOPTIMAL_KHR:                                         return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.(VK_SUBOPTIMAL_KHR)";
            case VK_ERROR_OUT_OF_DATE_KHR:                                  return "A surface has been changed in such a way that it is no longer compatible with the swapchainm and further presentation requests using the swapchain will fail. (VK_ERROR_OUT_OF_DATE_KHR)";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                         return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)";
            case VK_ERROR_VALIDATION_FAILED_EXT:                            return "A command failed because invalid usage was detected by the implementation or a validation-layer. (VK_ERROR_VALIDATION_FAILED_EXT)";
            case VK_ERROR_INVALID_SHADER_NV:                                return "One or more shaders failed to compile or link. (VK_ERROR_INVALID_SHADER_NV)";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:                    return "The requested VkImageUsageFlags are not supported. (VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:           return "The requested video picture layout is not supported. (VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:        return "A video profile operation specified via VkVideoProfileInfoKHR::videoCodecOperation is not supported. (VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:           return "Format parameters in a requested VkVideoProfileInfoKHR chain are not supported. (VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:            return "Codec-specific parameters in a requested VkVideoProfileInfoKHR chain are not supported. (VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:              return "The specified video Std header version is not supported. (VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:     return "(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)";
            case VK_ERROR_NOT_PERMITTED_KHR:                                return "(VK_ERROR_NOT_PERMITTED_KHR)";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:              return "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exclusive full-screen access. (VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)";
            case VK_THREAD_IDLE_KHR:                                        return "A deferred operation is not complete but there is currently no work for this thread to do at the time of this call. (VK_THREAD_IDLE_KHR)";
            case VK_THREAD_DONE_KHR:                                        return "A deferred operation is not complete but there is no work remaining to assign to additional threads. (VK_THREAD_DONE_KHR)";
            case VK_OPERATION_DEFERRED_KHR:                                 return "A deferred operation was requested and at least some of the work was deferred. (VK_OPERATION_DEFERRED_KHR)";
            case VK_OPERATION_NOT_DEFERRED_KHR:                             return "A deferred operation was requested and no operations were deferred. (VK_OPERATION_NOT_DEFERRED_KHR)";
            case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:                 return "The specified Video Std parameters do not adhere to the syntactic or semantic requirements of the used video compression standard, or values derived from parameters according to the rules defined by the used video compression standard do not adhere to the capabilities of the video compression standard or the implementation. (VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR)";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:                        return " An image creation failed because internal resources required for compression are exhausted. (VK_ERROR_COMPRESSION_EXHAUSTED_EXT)";
            case VK_INCOMPATIBLE_SHADER_BINARY_EXT:                         return "The provided binary shader code is not compatible with this device. (VK_INCOMPATIBLE_SHADER_BINARY_EXT)";
            case VK_PIPELINE_BINARY_MISSING_KHR:                            return "The application attempted to create a pipeline binary by querying an internal cache, but the internal cache entry did not exist. (VK_PIPELINE_BINARY_MISSING_KHR)";
            case VK_ERROR_NOT_ENOUGH_SPACE_KHR:                             return "The application did not provide enough space to return all the required data. (VK_ERROR_NOT_ENOUGH_SPACE_KHR)";
            default:                                                        return "Unknown Vulkan Error";
        }
    }

    /**
     * @brief GetVkSurfaceTransformsFlagString()
     * @param bits 
     * @return 
     */
    std::string GetVkSurfaceTransformsFlagString(uint32_t bits)
    {
        std::string out_string;
#define ADD_IF_CONDITION(x) \
        if(bits & x) { out_string = out_string + "\t\t" + #x + "\n";}

        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR);
        ADD_IF_CONDITION(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);

#undef ADD_IF_CONDITION

        return out_string;
    }

    /**
     * @brief GetVkCompositeAlphaFlagString()
     * @param bits 
     * @return 
     */
    std::string GetVkCompositeAlphaFlagString(uint32_t bits)
    {
        std::string out_string;
#define ADD_IF_CONDITION(x) \
        if(bits & x) { out_string = out_string + "\t\t" + #x + "\n";}

        ADD_IF_CONDITION(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        ADD_IF_CONDITION(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
        ADD_IF_CONDITION(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR);
        ADD_IF_CONDITION(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);

#undef ADD_IF_CONDITION

        return out_string;
    }

    /**
     * @brief GetVkImageUsageFlagsString()
     * @param bits 
     * @return 
     */
    std::string GetVkImageUsageFlagsString(uint32_t bits)
    {
        std::string out_string;
#define ADD_IF_CONDITION(x) \
        if(bits & x) { out_string = out_string + "\t\t" + #x + "\n";}

        ADD_IF_CONDITION(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_SAMPLED_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_STORAGE_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM);
        ADD_IF_CONDITION(VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM);

#undef ADD_IF_CONDITION

        return out_string;
    }

    /**
     * @brief GetVkFormatString()
     * @param value 
     * @return 
     */
    const char* GetVkFormatString(uint32_t value)
    {
        // code
#define CASE(x) case x: return #x;

#define CASE_ALIAS(x, alias) case x: return #x " | " #alias;
        
        switch(value)
        {
                CASE(VK_FORMAT_UNDEFINED);
                CASE(VK_FORMAT_R4G4_UNORM_PACK8);
                CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
                CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
                CASE(VK_FORMAT_R5G6B5_UNORM_PACK16);
                CASE(VK_FORMAT_B5G6R5_UNORM_PACK16);
                CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
                CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16);
                CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16);
                CASE(VK_FORMAT_R8_UNORM);
                CASE(VK_FORMAT_R8_SNORM);
                CASE(VK_FORMAT_R8_USCALED);
                CASE(VK_FORMAT_R8_SSCALED);
                CASE(VK_FORMAT_R8_UINT);
                CASE(VK_FORMAT_R8_SINT);
                CASE(VK_FORMAT_R8_SRGB);
                CASE(VK_FORMAT_R8G8_UNORM);
                CASE(VK_FORMAT_R8G8_SNORM);
                CASE(VK_FORMAT_R8G8_USCALED);
                CASE(VK_FORMAT_R8G8_SSCALED);
                CASE(VK_FORMAT_R8G8_UINT);
                CASE(VK_FORMAT_R8G8_SINT);
                CASE(VK_FORMAT_R8G8_SRGB);
                CASE(VK_FORMAT_R8G8B8_UNORM);
                CASE(VK_FORMAT_R8G8B8_SNORM);
                CASE(VK_FORMAT_R8G8B8_USCALED);
                CASE(VK_FORMAT_R8G8B8_SSCALED);
                CASE(VK_FORMAT_R8G8B8_UINT);
                CASE(VK_FORMAT_R8G8B8_SINT);
                CASE(VK_FORMAT_R8G8B8_SRGB);
                CASE(VK_FORMAT_B8G8R8_UNORM);
                CASE(VK_FORMAT_B8G8R8_SNORM);
                CASE(VK_FORMAT_B8G8R8_USCALED);
                CASE(VK_FORMAT_B8G8R8_SSCALED);
                CASE(VK_FORMAT_B8G8R8_UINT);
                CASE(VK_FORMAT_B8G8R8_SINT);
                CASE(VK_FORMAT_B8G8R8_SRGB);
                CASE(VK_FORMAT_R8G8B8A8_UNORM);
                CASE(VK_FORMAT_R8G8B8A8_SNORM);
                CASE(VK_FORMAT_R8G8B8A8_USCALED);
                CASE(VK_FORMAT_R8G8B8A8_SSCALED);
                CASE(VK_FORMAT_R8G8B8A8_UINT);
                CASE(VK_FORMAT_R8G8B8A8_SINT);
                CASE(VK_FORMAT_R8G8B8A8_SRGB);
                CASE(VK_FORMAT_B8G8R8A8_UNORM);
                CASE(VK_FORMAT_B8G8R8A8_SNORM);
                CASE(VK_FORMAT_B8G8R8A8_USCALED);
                CASE(VK_FORMAT_B8G8R8A8_SSCALED);
                CASE(VK_FORMAT_B8G8R8A8_UINT);
                CASE(VK_FORMAT_B8G8R8A8_SINT);
                CASE(VK_FORMAT_B8G8R8A8_SRGB);
                CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32);
                CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32);
                CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32);
                CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32);
                CASE(VK_FORMAT_R16_UNORM);
                CASE(VK_FORMAT_R16_SNORM);
                CASE(VK_FORMAT_R16_USCALED);
                CASE(VK_FORMAT_R16_SSCALED);
                CASE(VK_FORMAT_R16_UINT);
                CASE(VK_FORMAT_R16_SINT);
                CASE(VK_FORMAT_R16_SFLOAT);
                CASE(VK_FORMAT_R16G16_UNORM);
                CASE(VK_FORMAT_R16G16_SNORM);
                CASE(VK_FORMAT_R16G16_USCALED);
                CASE(VK_FORMAT_R16G16_SSCALED);
                CASE(VK_FORMAT_R16G16_UINT);
                CASE(VK_FORMAT_R16G16_SINT);
                CASE(VK_FORMAT_R16G16_SFLOAT);
                CASE(VK_FORMAT_R16G16B16_UNORM);
                CASE(VK_FORMAT_R16G16B16_SNORM);
                CASE(VK_FORMAT_R16G16B16_USCALED);
                CASE(VK_FORMAT_R16G16B16_SSCALED);
                CASE(VK_FORMAT_R16G16B16_UINT);
                CASE(VK_FORMAT_R16G16B16_SINT);
                CASE(VK_FORMAT_R16G16B16_SFLOAT);
                CASE(VK_FORMAT_R16G16B16A16_UNORM);
                CASE(VK_FORMAT_R16G16B16A16_SNORM);
                CASE(VK_FORMAT_R16G16B16A16_USCALED);
                CASE(VK_FORMAT_R16G16B16A16_SSCALED);
                CASE(VK_FORMAT_R16G16B16A16_UINT);
                CASE(VK_FORMAT_R16G16B16A16_SINT);
                CASE(VK_FORMAT_R16G16B16A16_SFLOAT);
                CASE(VK_FORMAT_R32_UINT);
                CASE(VK_FORMAT_R32_SINT);
                CASE(VK_FORMAT_R32_SFLOAT);
                CASE(VK_FORMAT_R32G32_UINT);
                CASE(VK_FORMAT_R32G32_SINT);
                CASE(VK_FORMAT_R32G32_SFLOAT);
                CASE(VK_FORMAT_R32G32B32_UINT);
                CASE(VK_FORMAT_R32G32B32_SINT);
                CASE(VK_FORMAT_R32G32B32_SFLOAT);
                CASE(VK_FORMAT_R32G32B32A32_UINT);
                CASE(VK_FORMAT_R32G32B32A32_SINT);
                CASE(VK_FORMAT_R32G32B32A32_SFLOAT);
                CASE(VK_FORMAT_R64_UINT);
                CASE(VK_FORMAT_R64_SINT);
                CASE(VK_FORMAT_R64_SFLOAT);
                CASE(VK_FORMAT_R64G64_UINT);
                CASE(VK_FORMAT_R64G64_SINT);
                CASE(VK_FORMAT_R64G64_SFLOAT);
                CASE(VK_FORMAT_R64G64B64_UINT);
                CASE(VK_FORMAT_R64G64B64_SINT);
                CASE(VK_FORMAT_R64G64B64_SFLOAT);
                CASE(VK_FORMAT_R64G64B64A64_UINT);
                CASE(VK_FORMAT_R64G64B64A64_SINT);
                CASE(VK_FORMAT_R64G64B64A64_SFLOAT);
                CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
                CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
                CASE(VK_FORMAT_D16_UNORM);
                CASE(VK_FORMAT_X8_D24_UNORM_PACK32);
                CASE(VK_FORMAT_D32_SFLOAT);
                CASE(VK_FORMAT_S8_UINT);
                CASE(VK_FORMAT_D16_UNORM_S8_UINT);
                CASE(VK_FORMAT_D24_UNORM_S8_UINT);
                CASE(VK_FORMAT_D32_SFLOAT_S8_UINT);
                CASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK);
                CASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK);
                CASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
                CASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
                CASE(VK_FORMAT_BC2_UNORM_BLOCK);
                CASE(VK_FORMAT_BC2_SRGB_BLOCK);
                CASE(VK_FORMAT_BC3_UNORM_BLOCK);
                CASE(VK_FORMAT_BC3_SRGB_BLOCK);
                CASE(VK_FORMAT_BC4_UNORM_BLOCK);
                CASE(VK_FORMAT_BC4_SNORM_BLOCK);
                CASE(VK_FORMAT_BC5_UNORM_BLOCK);
                CASE(VK_FORMAT_BC5_SNORM_BLOCK);
                CASE(VK_FORMAT_BC6H_UFLOAT_BLOCK);
                CASE(VK_FORMAT_BC6H_SFLOAT_BLOCK);
                CASE(VK_FORMAT_BC7_UNORM_BLOCK);
                CASE(VK_FORMAT_BC7_SRGB_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
                CASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
                CASE(VK_FORMAT_EAC_R11_UNORM_BLOCK);
                CASE(VK_FORMAT_EAC_R11_SNORM_BLOCK);
                CASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
                CASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK);
                CASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK);
                CASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK);

                CASE_ALIAS(VK_FORMAT_G8B8G8R8_422_UNORM, VK_FORMAT_G8B8G8R8_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_B8G8R8G8_422_UNORM, VK_FORMAT_B8G8R8G8_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_R10X6_UNORM_PACK16, VK_FORMAT_R10X6_UNORM_PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_R10X6G10X6_UNORM_2PACK16, VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_R12X4_UNORM_PACK16, VK_FORMAT_R12X4_UNORM_PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_R12X4G12X4_UNORM_2PACK16, VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR);
                CASE_ALIAS(VK_FORMAT_G16B16G16R16_422_UNORM, VK_FORMAT_G16B16G16R16_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_B16G16R16G16_422_UNORM, VK_FORMAT_B16G16R16G16_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR);
                CASE_ALIAS(VK_FORMAT_G8_B8R8_2PLANE_444_UNORM, VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT);
                CASE_ALIAS(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT);
                CASE_ALIAS(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16, VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT);
                CASE_ALIAS(VK_FORMAT_G16_B16R16_2PLANE_444_UNORM, VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT);
                CASE_ALIAS(VK_FORMAT_A4R4G4B4_UNORM_PACK16, VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT);
                CASE_ALIAS(VK_FORMAT_A4B4G4R4_UNORM_PACK16, VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT);

                CASE_ALIAS(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK, VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK, VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK, VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK, VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK, VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT);
                CASE_ALIAS(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK, VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT);
                CASE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG);
                CASE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
                CASE_ALIAS(VK_FORMAT_R16G16_SFIXED5_NV, VK_FORMAT_R16G16_S10_5_NV);
                CASE(VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR);
                CASE(VK_FORMAT_A8_UNORM_KHR);

                default:       return "UNDEFINED";
        }
        
#undef CASE
#undef CASE_ALIAS
    }

    /**
     * @brief GetVkColorSpaceString()
     * @param value 
     * @return 
     */
    const char* GetVkColorSpaceString(uint32_t value)
    {
#define ADD_IF_CONDITION(x) \
        if(value == x) { return #x;}

#define ADD_IF_CONDITION_2(x, alias_name) \
        if(value == x) { return #x ", " #alias_name;}
    
        ADD_IF_CONDITION_2(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_COLORSPACE_SRGB_NONLINEAR_KHR);
        ADD_IF_CONDITION(VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT);
        ADD_IF_CONDITION_2(VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT, VK_COLOR_SPACE_DCI_P3_LINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_BT709_LINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_BT709_NONLINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_BT2020_LINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_HDR10_ST2084_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_DOLBYVISION_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_HDR10_HLG_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_PASS_THROUGH_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT);
        ADD_IF_CONDITION(VK_COLOR_SPACE_DISPLAY_NATIVE_AMD);
        
        return "UNDEFINED";

#undef ADD_IF_CONDITION
#undef ADD_IF_CONDITION_2
    }

    /**
     * @brief GetVkPresentModeString()
     * @param value 
     * @return 
     */
    const char* GetVkPresentModeString(uint32_t value)
    {
#define ADD_IF_CONDITION(x) \
        if(value == x) { return #x;}

        ADD_IF_CONDITION(VK_PRESENT_MODE_IMMEDIATE_KHR);
        ADD_IF_CONDITION(VK_PRESENT_MODE_MAILBOX_KHR);
        ADD_IF_CONDITION(VK_PRESENT_MODE_FIFO_KHR);
        ADD_IF_CONDITION(VK_PRESENT_MODE_FIFO_RELAXED_KHR);
        ADD_IF_CONDITION(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
        ADD_IF_CONDITION(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);

        return "UNDEFINED";
#undef ADD_IF_CONDITION
    }

    /**
     * @brief GetVkObjectTypeName()
     * @param value 
     * @return 
     */
    const char* GetVkObjectTypeName(VkObjectType type)
    {
#define CASE(value) case value: return #value;
#define CASE_ALIAS(value, value2) case value: return #value ## " | " ## #value2;

        switch(type)
        {
            CASE(VK_OBJECT_TYPE_INSTANCE);
            CASE(VK_OBJECT_TYPE_PHYSICAL_DEVICE);
            CASE(VK_OBJECT_TYPE_DEVICE);
            CASE(VK_OBJECT_TYPE_QUEUE);
            CASE(VK_OBJECT_TYPE_SEMAPHORE);
            CASE(VK_OBJECT_TYPE_COMMAND_BUFFER);
            CASE(VK_OBJECT_TYPE_FENCE);
            CASE(VK_OBJECT_TYPE_DEVICE_MEMORY);
            CASE(VK_OBJECT_TYPE_BUFFER);
            CASE(VK_OBJECT_TYPE_IMAGE);
            CASE(VK_OBJECT_TYPE_EVENT);
            CASE(VK_OBJECT_TYPE_QUERY_POOL);
            CASE(VK_OBJECT_TYPE_BUFFER_VIEW);
            CASE(VK_OBJECT_TYPE_IMAGE_VIEW);
            CASE(VK_OBJECT_TYPE_SHADER_MODULE);
            CASE(VK_OBJECT_TYPE_PIPELINE_CACHE);
            CASE(VK_OBJECT_TYPE_PIPELINE_LAYOUT);
            CASE(VK_OBJECT_TYPE_RENDER_PASS);
            CASE(VK_OBJECT_TYPE_PIPELINE);
            CASE(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
            CASE(VK_OBJECT_TYPE_SAMPLER);
            CASE(VK_OBJECT_TYPE_DESCRIPTOR_POOL);
            CASE(VK_OBJECT_TYPE_DESCRIPTOR_SET);
            CASE(VK_OBJECT_TYPE_FRAMEBUFFER);
            CASE(VK_OBJECT_TYPE_COMMAND_POOL);
            CASE_ALIAS(VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION, VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR);
            CASE_ALIAS(VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR);
            CASE_ALIAS(VK_OBJECT_TYPE_PRIVATE_DATA_SLOT, VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT);
            CASE(VK_OBJECT_TYPE_SURFACE_KHR);
            CASE(VK_OBJECT_TYPE_SWAPCHAIN_KHR);
            CASE(VK_OBJECT_TYPE_DISPLAY_KHR);
            CASE(VK_OBJECT_TYPE_DISPLAY_MODE_KHR);
            CASE(VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT);
            CASE(VK_OBJECT_TYPE_VIDEO_SESSION_KHR);
            CASE(VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR);
            CASE(VK_OBJECT_TYPE_CU_MODULE_NVX);
            CASE(VK_OBJECT_TYPE_CU_FUNCTION_NVX);
            CASE(VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT);
            CASE(VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
            CASE(VK_OBJECT_TYPE_VALIDATION_CACHE_EXT);
            CASE(VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
            CASE(VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL);
            CASE(VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR);
            CASE(VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV);
            CASE(VK_OBJECT_TYPE_CUDA_MODULE_NV);
            CASE(VK_OBJECT_TYPE_CUDA_FUNCTION_NV);
            CASE(VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA);
            CASE(VK_OBJECT_TYPE_MICROMAP_EXT);
            CASE(VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV);
            CASE(VK_OBJECT_TYPE_SHADER_EXT);
            CASE(VK_OBJECT_TYPE_PIPELINE_BINARY_KHR);
            CASE(VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT);
            CASE(VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT);
            // CASE(VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE,
            // CASE(VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION,
            // CASE(VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT,

            default: return "VK_OBJECT_TYPE_UNKNOWN";
        }

#undef CASE
#undef CASE_ALIAS
    }
} // namespace VulkanHelper
