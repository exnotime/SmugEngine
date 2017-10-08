#include "ShaderLoader.h"
#include <json.hpp>
#include <shaderc/shaderc.h>
#include <fstream>
#include <Utility/Hash.h>
ShaderLoader::ShaderLoader() {}
ShaderLoader::~ShaderLoader() {}


ShaderByteCode* CompileShader(const std::string& file, SHADER_KIND kind, const std::string& entryPoint, SHADER_LANGUAGE lang, bool debug) {
	//load shader file
	struct stat fileBuf;
	stat(file.c_str(), &fileBuf);

	FILE* fin = fopen(file.c_str(), "rb");
	char* code = new char[fileBuf.st_size];
	fread(code, sizeof(char), fileBuf.st_size, fin);
	fclose(fin);

	//compile into spir-v
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();

	if (lang == GLSL)
		shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
	else if (lang == HLSL)
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);

	if(debug)
		shaderc_compile_options_set_generate_debug_info(options);
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level::shaderc_optimization_level_zero);

	auto includeResolver = [](void* user_data, const char* requested_source, int type,
		const char* requesting_source, size_t include_depth) -> shaderc_include_result* {
		std::string filename;
		if (type == shaderc_include_type_relative) {
			std::string reqSrc = std::string(requesting_source);
			filename = reqSrc.substr(0, reqSrc.find_last_of('/')) + '/' + std::string(requested_source);
		}
		else if (type == shaderc_include_type_standard) {
			filename = "shaders/" + std::string(requested_source);
		}
		FILE* fin = fopen(filename.c_str(), "rb");
		fseek(fin, 0, SEEK_END);
		size_t size = ftell(fin);
		rewind(fin);
		char* src = new char[size];
		fread(src, sizeof(char), size, fin);
		fclose(fin);

		char* srcfile = new char[filename.size()];
		strcpy(srcfile, filename.c_str());

		std::vector<std::string>* includes = (std::vector<std::string>*)user_data;
		includes->push_back(filename);

		shaderc_include_result* res = new shaderc_include_result();
		res->content = src;
		res->content_length = size;
		res->source_name = srcfile;
		res->source_name_length = filename.size();
		res->user_data = nullptr;
		return res;
	};

	auto includeResRelease = [](void* user_data, shaderc_include_result* include_result) {
		delete include_result;
	};
	//used to keep track of dependences
	std::vector<std::string> includes;
	shaderc_compile_options_set_include_callbacks(options, includeResolver, includeResRelease, &includes);

	shaderc_shader_kind shaderType;
	if (kind == VERTEX) {
		shaderType = shaderc_glsl_vertex_shader;
	}else if (kind == FRAGMENT) {
		shaderType = shaderc_glsl_fragment_shader;
	}else if (kind == GEOMETRY) {
		shaderType = shaderc_glsl_geometry_shader;
	}else if (kind == CONTROL) {
		shaderType = shaderc_glsl_tess_control_shader;
	}else if (kind == EVALUATION) {
		shaderType = shaderc_glsl_tess_evaluation_shader;
	}else if (kind == COMPUTE) {
		shaderType = shaderc_glsl_compute_shader;
	}

	shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, code, fileBuf.st_size, shaderType, file.c_str(), "main", options);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
		const char* errors = shaderc_result_get_error_message(result);
		shaderc_result_release(result);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
		return nullptr;
	}
	delete[] code;
	ShaderByteCode* bc = new ShaderByteCode();
	bc->ByteCodeSize = shaderc_result_get_length(result);
	bc->Kind = kind;
	bc->SrcLanguage = lang;
	bc->Type = SPIRV;
	bc->ByteCode = malloc(bc->ByteCodeSize);
	memcpy(bc->ByteCode, shaderc_result_get_bytes(result), bc->ByteCodeSize);
	bc->DependencyCount = includes.size();
	if(bc->DependencyCount){
		bc->DependenciesHashes = (uint32_t*)malloc(sizeof(uint32_t) * bc->DependencyCount);
		for (int i = 0; i < bc->DependencyCount; ++i) {
			bc->DependenciesHashes[i] = HashString(includes[i]);
		}
	}

	//clean up
	shaderc_result_release(result);
	shaderc_compile_options_release(options);
	shaderc_compiler_release(compiler);

	return bc;
}

char* ShaderLoader::LoadShaders(const std::string& filename, ShaderInfo& info) {
	using namespace nlohmann;

	std::ifstream fin(filename);
	if (!fin.is_open()) {
		//printf("Error opening pipeline file %s\n", filename.c_str());
		fin.close();
		return "Could not open shader file";
	}
	json root;
	try {
		fin >> root;
	}
	catch (std::exception e) {
		printf("json: %s\n", e.what());
		return " json error";
	}
	fin.close();
	//Find shaders
	if (root.find("Shaders") != root.end()) {
		json shaders = root["Shaders"];
		std::vector<ShaderByteCode*> byte_codes;
		std::string shader_types[] = { "Vertex", "Fragment", "Geometry", "Evaluation", "Control", "Compute" };
		for (int i = 0; i <= COMPUTE; ++i) {
			if (shaders.find(shader_types[i]) != shaders.end()) {
				json shader = shaders[shader_types[i]];
				std::string entry;
				SHADER_LANGUAGE lang = GLSL;

				if (shader.find("EntryPoint") != shader.end())
					entry = shader["EntryPoint"];

				if (shader.find("Language") != shader.end()) {
					if (shader["Language"] == "GLSL") {
						lang = GLSL;
					}
					else if (shader["Language"] == "HLSL") {
						lang = HLSL;
					}
				}
				if (entry.empty())
					entry = "main";

				ShaderByteCode* bc = CompileShader(shader["Source"], SHADER_KIND(i), entry, lang, false);
				if (!bc) {
					return "Failed to compile shaders";
				}
				byte_codes.push_back(bc);
			}
		}
		info.ShaderCount = byte_codes.size();
		info.Shaders = new ShaderByteCode[info.ShaderCount];
		for (int i = 0; i < info.ShaderCount; ++i) {
			info.Shaders[i] = *byte_codes[i];
		}

	} else {
		return "No Shaders found in File";
	}
	return nullptr;
}