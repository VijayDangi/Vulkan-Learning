cl /EHsc Main.cpp VkApplication.cpp VulkanHelper.cpp ^
   /I"%VK_SDK_PATH%\Include" ^
   /link /OUT:App.exe /LIBPATH:"%VK_SDK_PATH%\Lib" ^
   user32.lib gdi32.lib vulkan-1.lib

del Main.obj ^
    VkApplication.obj ^
    VulkanHelper.obj

