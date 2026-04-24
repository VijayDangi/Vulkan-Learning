
echo off
cls

cl /EHsc /I"%VK_SDK_PATH%\Include" /I"..\Common\stb" ^
    Main.cpp ^
    VkApplication.cpp ^
    VulkanHelper.cpp ^
    /link /OUT:App.exe /LIBPATH:"%VK_SDK_PATH%\Lib" ^
    user32.lib gdi32.lib vulkan-1.lib


del Main.obj
del VkApplication.obj
del VulkanHelper.obj

echo on