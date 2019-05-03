set VULKAN_SDK_PATH="C:\VulkanSDK\1.1.106.0"
set GLSL_LANG_VALIDATOR=%VULKAN_SDK_PATH%"\Bin32\glslangValidator.exe"

%GLSL_LANG_VALIDATOR% -V shader.vert
%GLSL_LANG_VALIDATOR% -V shader.frag

pause
