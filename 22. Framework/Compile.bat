SET COMPILER_PATH=%VK_SDK_PATH%\Bin

@REM %COMPILER_PATH%\glslc.exe shaders\triangle.vert -o shaders\triangle.vert.spv
@REM %COMPILER_PATH%\glslc.exe shaders\triangle.frag -o shaders\triangle.frag.spv

%COMPILER_PATH%\glslangValidator.exe -V shaders\advect.comp -o shaders\advect.comp.spv
%COMPILER_PATH%\glslangValidator.exe -V shaders\render.vert -o shaders\render.vert.spv
%COMPILER_PATH%\glslangValidator.exe -V shaders\render.frag -o shaders\render.frag.spv
