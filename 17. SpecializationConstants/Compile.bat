SET COMPILER_PATH=%VK_SDK_PATH%\Bin

@REM %COMPILER_PATH%\glslc.exe shaders\triangle.vert -o shaders\triangle.vert.spv
@REM %COMPILER_PATH%\glslc.exe shaders\triangle.frag -o shaders\triangle.frag.spv

%COMPILER_PATH%\glslangValidator.exe -V shaders\vertexShader.vert -o shaders\vertexShader.vert.spv
%COMPILER_PATH%\glslangValidator.exe -V shaders\fragmentShader.frag -o shaders\fragmentShader.frag.spv
