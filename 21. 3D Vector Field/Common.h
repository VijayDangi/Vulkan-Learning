#ifndef __COMMON_H__
#define __COMMON_H__

#include <Windows.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
    #define IMGUI_DEFINE_MATH_OPERATORS
#endif

#define VK_USE_PLATFORM_WIN32_KHR   // Used to include <vulkan/vulkan_win32.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_vulkan.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VX_FRAMEWORK_NAMESPACE vxs

#define VK_UTIL_FN_ERROR_CHECK(x) \
    {   \
        VkResult errorCode = x; \
        if( errorCode != VK_SUCCESS)    \
        {   \
            LogError("Vulkan: [Error] " #x " Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(errorCode));   \
        }   \
    }

#define VK_UTIL_FN_ERROR_CHECK_RETURN(x, return_on_failed) \
    {   \
        VkResult errorCode = x; \
        if( errorCode != VK_SUCCESS)    \
        {   \
            LogError("Vulkan: [Error] " #x " Failed. %s", VX_FRAMEWORK_NAMESPACE::helper::GetVulkanErrorCodeString(errorCode));   \
            return return_on_failed;    \
        }   \
    }


#define LOG_CONSOLE_TEXT_WHITE_COLOR    0
#define LOG_CONSOLE_BKG_BLACK_COLOR     0

#define LOG_CONSOLE_DARK_TEXT_RED_COLOR      31
#define LOG_CONSOLE_DARK_TEXT_GREEN_COLOR    32
#define LOG_CONSOLE_DARK_TEXT_YELLOW_COLOR   33
#define LOG_CONSOLE_DARK_TEXT_BLUE_COLOR     34
#define LOG_CONSOLE_DARK_TEXT_MAGINTA_COLOR  35
#define LOG_CONSOLE_DARK_TEXT_CYAN_COLOR     36

#define LOG_CONSOLE_LIGHT_TEXT_GREY_COLOR     90
#define LOG_CONSOLE_LIGHT_TEXT_RED_COLOR      91
#define LOG_CONSOLE_LIGHT_TEXT_GREEN_COLOR    92
#define LOG_CONSOLE_LIGHT_TEXT_YELLOW_COLOR   93
#define LOG_CONSOLE_LIGHT_TEXT_BLUE_COLOR     94
#define LOG_CONSOLE_LIGHT_TEXT_MAGINTA_COLOR  95
#define LOG_CONSOLE_LIGHT_TEXT_CYAN_COLOR     96

#define LOG_CONSOLE_DARK_BKG_RED_COLOR      41
#define LOG_CONSOLE_DARK_BKG_GREEN_COLOR    42
#define LOG_CONSOLE_DARK_BKG_YELLOW_COLOR   43
#define LOG_CONSOLE_DARK_BKG_BLUE_COLOR     44
#define LOG_CONSOLE_DARK_BKG_MAGINTA_COLOR  45
#define LOG_CONSOLE_DARK_BKG_CYAN_COLOR     46
#define LOG_CONSOLE_DARK_BKG_GREY_COLOR     47

#define LOG_CONSOLE_LIGHT_BKG_GREY_COLOR     100
#define LOG_CONSOLE_LIGHT_BKG_RED_COLOR      101
#define LOG_CONSOLE_LIGHT_BKG_GREEN_COLOR    102
#define LOG_CONSOLE_LIGHT_BKG_YELLOW_COLOR   103
#define LOG_CONSOLE_LIGHT_BKG_BLUE_COLOR     104
#define LOG_CONSOLE_LIGHT_BKG_MAGINTA_COLOR  105
#define LOG_CONSOLE_LIGHT_BKG_CYAN_COLOR     106
#define LOG_CONSOLE_LIGHT_BKG_WHITE_COLOR    107


#define Log(fmt, ...) \
        PrintLog(LOG_CONSOLE_LIGHT_TEXT_GREY_COLOR, LOG_CONSOLE_BKG_BLACK_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LogError(fmt, ...) \
        PrintLog(LOG_CONSOLE_DARK_TEXT_RED_COLOR, LOG_CONSOLE_BKG_BLACK_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LogDebug(fmt, ...) \
        PrintLog(LOG_CONSOLE_DARK_TEXT_GREEN_COLOR, LOG_CONSOLE_LIGHT_BKG_WHITE_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LogWarning(fmt, ...) \
        PrintLog(LOG_CONSOLE_LIGHT_TEXT_YELLOW_COLOR, LOG_CONSOLE_BKG_BLACK_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LogInfo(fmt, ...) \
        PrintLog(LOG_CONSOLE_LIGHT_TEXT_BLUE_COLOR, LOG_CONSOLE_BKG_BLACK_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

#define LogSuccess(fmt, ...) \
        PrintLog(LOG_CONSOLE_LIGHT_TEXT_GREEN_COLOR, LOG_CONSOLE_BKG_BLACK_COLOR, __LINE__, __FILE__, __FUNCTION__, fmt, __VA_ARGS__)

void PrintLog( int text_color, int background_color, int lineNo, char *fileName, char *functionName, char *format, ...);


extern HWND  ghwnd;

#endif // _COMMON_H__