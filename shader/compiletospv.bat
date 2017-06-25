set arg=%1
set arg=%arg:"=%
%VULKAN_SDK%\Bin\glslangValidator.exe -V -o spv\%arg%.spv %arg%