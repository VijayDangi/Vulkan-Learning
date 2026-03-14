echo off

cls

cl /EHsc /I"%VK_SDK_PATH%\Include" /I"..\Common\ktx\include" /I"..\Common\tiny_gltf"^
    Main.cpp ^
    VkApplication.cpp ^
    VulkanHelper.cpp ^
    /link /OUT:App.exe /LIBPATH:"%VK_SDK_PATH%\Lib" /LIBPATH:"..\Common\ktx\lib" ^
    user32.lib gdi32.lib vulkan-1.lib ktx.lib

del Main.obj
del VkApplication.obj
del VulkanHelper.obj

copy ..\Common\ktx\bin\ktx.dll .

echo on