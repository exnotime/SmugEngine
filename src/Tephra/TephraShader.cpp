#include "TephraShader.h"
#include <shaderc/shaderc.h>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

vk::ShaderModule Tephra::LoadShader(const vk::Device& device, const std::string& filename) {
	//read file ending and figure out shader type
	size_t lastDot = filename.find_last_of('.');
	std::string fileEnding = filename.substr(lastDot + 1);
	shaderc_shader_kind shaderType;
	if (fileEnding == "vert") {
		shaderType = shaderc_glsl_vertex_shader;
	}
	else if (fileEnding == "frag") {
		shaderType = shaderc_glsl_fragment_shader;
	}
	else if (fileEnding == "geom") {
		shaderType = shaderc_glsl_geometry_shader;
	}
	else if (fileEnding == "tesc") {
		shaderType = shaderc_glsl_tess_control_shader;
	}
	else if (fileEnding == "tese") {
		shaderType = shaderc_glsl_tess_evaluation_shader;
	}
	else if (fileEnding == "comp") {
		shaderType = shaderc_glsl_compute_shader;
	}
	else if (fileEnding == "spv") {
		//Load  precompiled shader
		vk::ShaderModuleCreateInfo shaderInfo;
		FILE* fin = fopen(filename.c_str(), "rb");
		fseek(fin, 0, SEEK_END);
		uint64_t fileSize = ftell(fin);
		shaderInfo.codeSize = fileSize;
		rewind(fin);
		char* code = new char[fileSize];
		fread(code, sizeof(char), fileSize, fin);
		fclose(fin);
		shaderInfo.pCode = reinterpret_cast<const uint32_t*>(code);
		vk::ShaderModule module = device.createShaderModule(shaderInfo);
		delete[] code;
		return module;
	}

	//check if there is an up to date shader cache
	std::string cacheName = SHADER_CACHE_DIR + filename.substr(filename.find_last_of('/') + 1) + ".spv";
	struct stat cacheBuf, fileBuf;
	stat(cacheName.c_str(), &cacheBuf);
	stat(filename.c_str(), &fileBuf);
	//also checks if there is a cache
	if (cacheBuf.st_mtime >= fileBuf.st_mtime) {
		//there is an up to date cache
		vk::ShaderModuleCreateInfo shaderInfo;
		FILE* fin = fopen(cacheName.c_str(), "rb");
		shaderInfo.codeSize = cacheBuf.st_size;
		char* code = new char[cacheBuf.st_size];
		fread(code, sizeof(char), cacheBuf.st_size, fin);
		fclose(fin);
		shaderInfo.pCode = reinterpret_cast<const uint32_t*>(code);
		vk::ShaderModule module = device.createShaderModule(shaderInfo);
		delete[] code;
		return module;
	}
	//load shader file
	FILE* fin = fopen(filename.c_str(), "rb");
	char* code = new char[fileBuf.st_size];
	fread(code, sizeof(char), fileBuf.st_size, fin);
	fclose(fin);
	//compile into spir-v
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
#ifdef DEBUG
	shaderc_compile_options_set_generate_debug_info(options);
#endif
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level::shaderc_optimization_level_zero);

	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, code, fileBuf.st_size, shaderType, filename.c_str(), "main", options);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
		const char* errors = shaderc_result_get_error_message(result);
		printf("Error compiling shader %s \n Errors %s\n", filename.c_str(), errors);
		return nullptr;
	}
	delete[] code;
	//create vulkan shader module
	vk::ShaderModuleCreateInfo shaderInfo;
	shaderInfo.codeSize = shaderc_result_get_length(result);
	shaderInfo.pCode = reinterpret_cast<const uint32_t*>(shaderc_result_get_bytes(result));
	vk::ShaderModule module = device.createShaderModule(shaderInfo);
	//save to cache
	FILE* fout = fopen(cacheName.c_str(), "wb");
	fwrite(shaderc_result_get_bytes(result), sizeof(char), shaderc_result_get_length(result), fout);
	fclose(fout);
	//clean up
	shaderc_result_release(result);
	shaderc_compiler_release(compiler);
	return module;
}
