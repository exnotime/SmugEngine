/*
Copyright 2017-01-27 Henrik Johansson texturehandle@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHERLIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#include <vulkan/vulkan.hpp>

#ifndef SHADER_CACHE_DIR
//#define USE_SHADER_CACHE 1
#define SHADER_CACHE_DIR "./shader/cache/"
#endif



namespace smug {
enum SHADER_LANGUAGE {
	GLSL,
	HLSL
};

enum SHADER_STAGE {
	VERTEX,
	FRAGMENT,
	GEOMETRY,
	CONTROL,
	EVALUATION,
	COMPUTE,
	PRECOMPILED
};

vk::ShaderModule LoadShader(const vk::Device& device, const std::string& filename, SHADER_STAGE stage, const std::string& entryPoint = "main", SHADER_LANGUAGE language = GLSL);
};