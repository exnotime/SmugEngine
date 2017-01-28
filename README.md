# Tephra
C++ library for reading Vulkan pipelines from a json format. At the moment this library is windows only.
This is currently a work in progress.

Tephra consist of two parts:
* A pipeline reader that generates a vk::Pipeline from a json file.
* A shader compiler that compiles glsl code to spir-v and saves the resulting code in a cache.

#Pipeline reader
The pipeline reader reads a json file and generates a vk::pipeline object.
If a field is not specified a default value will be used instead. The default values can be seen shader/default.json.
The default values are designed for a pipeline state which could be used to render triangles in a triangle list to a color target.

Certain fields have no candidate for a default value and is therefore mandatory.
For example the fields in a DescriptorSetLayout needs to be set
{"Binding":0, "Count":2, "Type":"UniformBuffer"}

The files filled.json, frontface.json and wireframe.json contain some examples on how the data can be formatted.

Tephra uses https://github.com/nlohmann/json to parse the json file.
#Shader compiler
The shader compiler uses shaderc to compile glsl code to spir-v.
The shader files need to have a file ending similar to those used for glslangValidator.
If a file is supplied with the file ending .spv then the file will be assumed to contain spir-v code.
The compiled files will be cached, by default the files will be cached in the root folder.
The define SHADER_CACHE_DIR can be specified to change what directory is used to cache the files.
If a source file is updated the cache will be updated as well.

#The ExampleApp
The example app uses:

glfw3 (for windowing and input) : http://www.glfw.org/docs/latest/

glm (for mathematics) : http://glm.g-truc.net/0.9.8/index.html

par_shapes (to generate a mesh) : https://github.com/prideout/par/blob/master/par_shapes.h


* Compile shaderc for your system (available in the Vulkan SDK) and place shaderc_combined.lib (shaderc_combinedD.lib for debug mode) in the lib folder.
* Use GenerateSolution.sh to generate a .sln file.

In the example app use the left and right arrow keys to switch between three different pipeline states.
