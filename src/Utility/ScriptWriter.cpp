#include "ScriptWriter.h"
#include <cstdarg>
#include <fstream>
ScriptWriter::ScriptWriter(){
	//All scripts have a global scope
	m_CurrentScope = nullptr;
	Scope s;
	s.Name = "Global";
	s.Function = false;
	m_Scopes.push_back(s);
	m_GlobalScope = &m_Scopes.back();
	m_CurrentScope = m_GlobalScope;
	m_VariableData.reserve(4);
}

ScriptWriter::~ScriptWriter(){

}

void WriteScopeToFile(std::ofstream& fout, Scope& scope) {
	if (scope.Name != "Global") {
		//write out function header
		fout << VariableTypeNames[scope.ReturnVariable.Type];
		fout << " " << scope.Name.c_str() << "(";
		uint32_t inputVarCount = (uint32_t)scope.InputVariables.size();
		for (uint32_t i = 0; i < inputVarCount; ++i) {
			fout << VariableTypeNames[scope.InputVariables[i].Type] << " " << scope.InputVariables[i].Name.c_str();
			if (i != inputVarCount - 1) {
				fout << ", ";
			}
		}
		fout << "){" << std::endl;
	}

	//write out local variables
	uint32_t localVarCount = (uint32_t)scope.LocalVariables.size();
	for (uint32_t i = 0; i < localVarCount; ++i) {
		auto& var = scope.LocalVariables[i];
		fout << VariableTypeNames[var.Type] << " " << var.Name.c_str();
		if (var.Array) {
			fout << "[" << var.ArrayCount << "]";
		}
		if (var.Data) {
			fout << " = ";
			if (var.Array) {
				fout << "[" << std::endl;
			}
			for (uint32_t k = 0; k < var.ArrayCount; ++k) {
				switch (var.Type) {
				case VAR_INT:
					fout << ((int*)(var.Data))[k];
					break;
				case VAR_BOOL:
					fout << ((bool*)(var.Data))[k] ? "true" : "false";
					break;
				case VAR_FLOAT:
					fout << eastl::to_string(((float*)(var.Data))[k]).c_str();
					break;
				case VAR_VEC2:
					fout << "vec2(" << ((glm::vec2*)(var.Data))[k].x << "," << ((glm::vec2*)(var.Data))[k].y << ")";
					break;
				case VAR_VEC3:
					fout << "vec3(" << ((glm::vec3*)(var.Data))[k].x << "," << ((glm::vec3*)(var.Data))[k].y << "," << ((glm::vec3*)(var.Data))[k].z << ")";
					break;
				case VAR_VEC4:
					fout << "vec4(" << ((glm::vec4*)(var.Data))[k].x << "," << ((glm::vec4*)(var.Data))[k].y << "," << ((glm::vec4*)(var.Data))[k].z << "," << ((glm::vec4*)(var.Data))[k].w << ")";
					break;
				case VAR_QUAT:
					fout << "quat(" << ((glm::vec4*)(var.Data))[k].x << "," << ((glm::vec4*)(var.Data))[k].y << "," << ((glm::vec4*)(var.Data))[k].z << "," << ((glm::vec4*)(var.Data))[k].w << ")";
					break;
				}
				if (var.Array) {
					fout << "," << std::endl;
				}
			}
			if (var.Array) {
				fout << "]";
			}
		}
		fout << ";" << std::endl;
	}

	fout << scope.Code.c_str() << std::endl;
	if (scope.Function)
		fout << "}" << std::endl;
	fout << std::endl;

}

void ScriptWriter::WriteToFile(const char* file) {
	std::ofstream fout;
	fout.open(file, std::ofstream::out | std::ofstream::binary);
	if (fout.is_open()) {
		uint32_t scopeCount = (uint32_t)m_Scopes.size();
		for (uint32_t i = 0; i < scopeCount; ++i) {
			WriteScopeToFile(fout, m_Scopes[i]);
		}
		fout.close();
	}
}

void ScriptWriter::AddSnippet(const char* snippet) {
	m_CurrentScope->Code += snippet;
}

void ScriptWriter::AddVariable(const char* name, VariableType type) {
	Variable v;
	v.Type = type;
	v.Array = false;
	v.ArrayCount = 1;
	v.Name = name;
	v.Data = nullptr;
	m_CurrentScope->LocalVariables.push_back(v);
}

void ScriptWriter::AddVariable(const char* name, VariableType type, void* data) {
	Variable v;
	v.Type = type;
	v.Array = false;
	v.ArrayCount = 1;
	v.Name = name;
	v.Data = AllocateVariable(VariableTypeSize[type]);
	memcpy(v.Data, data, VariableTypeSize[type]);
	m_CurrentScope->LocalVariables.push_back(v);
}

void ScriptWriter::AddVariableArray(const char* name, VariableType type, uint32_t count, void* data) {
	Variable v;
	v.Type = type;
	v.Array = true;
	v.ArrayCount = count;
	v.Name = name;
	v.Data = AllocateVariable(VariableTypeSize[type] * count);
	memcpy(v.Data, data, VariableTypeSize[type] * count);
	m_CurrentScope->LocalVariables.push_back(v);
}

void ScriptWriter::OpenFunction(const char* name) {
	if (m_CurrentScope->Function) {
		//need to close the current function first
		return;
	}
	Scope s;
	s.Code = "";
	s.Name = name;
	s.Function = true;
	s.ReturnVariable.Type = VAR_VOID;
	m_Scopes.push_back(s);
	m_CurrentScope = &m_Scopes.back();
}

void ScriptWriter::AddReturnVariable(VariableType type) {
	if (!m_CurrentScope->Function) {
		return;
	}
	Variable v;
	v.Type = type;
	v.Name = "ReturnVar";
	m_CurrentScope->ReturnVariable = v;
}

void ScriptWriter::AddInputVariable(const char* name, VariableType type) {
	if (!m_CurrentScope) {
		return;
	}
	Variable v;
	v.Name = name;
	v.Type = type;
	m_CurrentScope->InputVariables.push_back(v);
}

void ScriptWriter::CloseFunction() {
	m_CurrentScope = m_GlobalScope;
}

void* ScriptWriter::AllocateVariable(uint32_t size) {
	int oldSize = (uint32_t)m_VariableData.size();
	void* end = (void*)(m_VariableData.data() + oldSize);
	m_VariableData.resize(oldSize + size);
	return end;
}