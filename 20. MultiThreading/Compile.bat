SET COMPILER_PATH=%VK_SDK_PATH%\Bin

@REM %COMPILER_PATH%\glslc.exe shaders\triangle.vert -o shaders\triangle.vert.spv
@REM %COMPILER_PATH%\glslc.exe shaders\triangle.frag -o shaders\triangle.frag.spv

%COMPILER_PATH%\glslangValidator.exe -V shaders\triangle.vert -o shaders\triangle.vert.spv
%COMPILER_PATH%\glslangValidator.exe -V shaders\triangle.frag -o shaders\triangle.frag.spv

%COMPILER_PATH%\glslangValidator.exe -V shaders\compute.comp -o shaders\compute.comp.spv

