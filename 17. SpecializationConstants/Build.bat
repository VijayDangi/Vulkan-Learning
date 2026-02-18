@echo off
setlocal
cls

echo Compiling And Linking...
echo.
cl /EHsc Main.cpp VkApplication.cpp VulkanHelper.cpp VkUtil.cpp ^
    /I"%VK_SDK_PATH%\Include" ^
    /I"../Common/ktx/include" ^
    /link /OUT:App.exe /LIBPATH:"%VK_SDK_PATH%\Lib" /LIBPATH:"..\Common\ktx\lib" ^
    user32.lib gdi32.lib vulkan-1.lib ktx.lib

IF ERRORLEVEL 1 (
    del  Main.obj VkApplication.obj VulkanHelper.obj VkUtil.obj
    exit /b 1
)

del  Main.obj VkApplication.obj VulkanHelper.obj VkUtil.obj

echo.
echo.
echo Copying DLL's from Common folder...
Copy /Y ..\Common\ktx\bin\*.dll .

endlocal
