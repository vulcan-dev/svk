@echo off

set VKBIN=F:/SDK/Vulkan/1.3.250.1/Bin
call %VKBIN%/glslc.exe ./rom/shaders/shader.vert -o ./rom/shaders/compiled/vert.spv
call %VKBIN%/glslc.exe ./rom/shaders/shader.frag -o ./rom/shaders/compiled/frag.spv