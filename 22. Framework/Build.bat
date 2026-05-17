del App.exe

cl /EHsc ^
 Main.cpp ^
 VkApplication.cpp ^
 vx_helpers.cpp ^
 vx_utils.cpp ^
 ..\Common\imgui\imgui.cpp ^
 ..\Common\imgui\imgui_draw.cpp ^
 ..\Common\imgui\imgui_tables.cpp ^
 ..\Common\imgui\imgui_widgets.cpp ^
 ..\Common\imgui\imgui_impl_win32.cpp ^
 ..\Common\imgui\imgui_impl_vulkan.cpp ^
 ..\Common\imgui\imgui_stdlib.cpp ^
 /I"%VK_SDK_PATH%\Include" ^
 /I"..\Common\stb" ^
 /I"..\Common\glm" ^
 /I"..\Common\imgui" ^
 /link /OUT:App.exe /LIBPATH:"%VK_SDK_PATH%\Lib" ^
 user32.lib gdi32.lib vulkan-1.lib

del Main.obj ^
 VkApplication.obj ^
 vx_helpers.obj ^
 vx_utils.obj ^
 imgui.obj ^
 imgui_draw.obj ^
 imgui_tables.obj ^
 imgui_widgets.obj ^
 imgui_impl_win32.obj ^
 imgui_impl_vulkan.obj ^
 imgui_stdlib.obj ^