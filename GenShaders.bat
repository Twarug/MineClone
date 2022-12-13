@Prompt $G
pushd run\assets\shaders

%VULKAN_SDK%\Bin\glslc.exe default.vert -o default.vert.spv
%VULKAN_SDK%\Bin\glslc.exe default.frag -o default.frag.spv

popd
PAUSE