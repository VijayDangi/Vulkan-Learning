#ifndef __VK_APPLICATION_H__
#define __VK_APPLICATION_H__

namespace VkApplication
{
    bool Initialize(HWND hwnd);
    void ResizeCallback(int width, int height);
    void DrawFrame();
    void Uninitialize();
    
} // namespace VkApplication

#endif // __VK_APPLICATION_H__
