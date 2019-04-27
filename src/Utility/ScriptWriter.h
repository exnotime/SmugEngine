#pragma once
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <glm/glm.hpp>

#define CHUNK_SIZE 4096

//Built in variable types
enum VariableType {
	VAR_VOID,
	VAR_INT,
	VAR_BOOL,
	VAR_STRING,
	VAR_FLOAT,
	VAR_VEC2,
	VAR_VEC3,
	VAR_VEC4,
	VAR_MAT3,
	VAR_MAT4,
	VARIABLE_TYPE_COUNT
};

//Must match the size of the enum VariableType
static uint32_t VariableTypeSize[VARIABLE_TYPE_COUNT] = {
	0,
	sizeof(int),
	sizeof(bool),
	sizeof(eastl::string),
	sizeof(float),
	sizeof(glm::vec2),
	sizeof(glm::vec3),
	sizeof(glm::vec4),
	sizeof(glm::mat3),
	sizeof(glm::mat4)
};

static const char* VariableTypeNames[VARIABLE_TYPE_COUNT] = {
	"void",
	"int",
	"bool",
	"string",
	"float",
	"vec2",
	"vec3",
	"vec4",
	"mat3",
	"mat4"
};

struct Variable {
	eastl::string Name;
	VariableType Type;
	void* Data;
	bool Array;
	uint32_t ArrayCount;

	void operator =(const Variable& rhs) {
		if (rhs.Type != this->Type)
			return;
		assert(!this->Array);
		memcpy(this->Data, rhs.Data, VariableTypeSize[this->Type]);
	}

	void operator =(const char* rhs) {
		assert(this->Type == VAR_STRING);
		*(eastl::string*)this->Data = rhs;
	}
	void operator =(const int rhs) {
		assert(this->Type == VAR_INT);
		*(int*)this->Data = rhs;
	}
	void operator =(const bool rhs) {
		assert(this->Type == VAR_BOOL);
		*(bool*)this->Data = rhs;
	}
	void operator =(const float rhs) {
		assert(this->Type == VAR_FLOAT);
		*(float*)this->Data = rhs;
	}
	void operator =(const glm::vec2& rhs) {
		assert(this->Type == VAR_VEC2);
		*(glm::vec2*)this->Data = rhs;
	}
	void operator =(const glm::vec3& rhs) {
		assert(this->Type == VAR_VEC3);
		*(glm::vec3*)this->Data = rhs;
	}
	void operator =(const glm::vec4& rhs) {
		assert(this->Type == VAR_VEC4);
		*(glm::vec4*)this->Data = rhs;
	}
	void operator =(const glm::mat3& rhs) {
		assert(this->Type == VAR_MAT3);
		*(glm::mat3*)this->Data = rhs;
	}
	void operator =(const glm::mat4& rhs) {
		assert(this->Type == VAR_MAT4);
		*(glm::mat4*)this->Data = rhs;
	}
};

struct Scope {
	eastl::string Name;
	Variable ReturnVariable;
	eastl::vector<Variable> InputVariables;
	eastl::vector<Variable> LocalVariables;
	eastl::string Code;
	bool Function;
};


//Helper class to programaticly write a script file
class ScriptWriter {
public:
	ScriptWriter();
	~ScriptWriter();
	void WriteToFile(const char* file);
	//General
	void IncludeLocalFile(const char* file);
	void IncludeGlobalFile(const char* file);
	void AddSnippet(const char* snippet);//writes the snippet into the end of the current scope
	void AddVariable(const char* name, VariableType type);//declares a variable without any initialization
	void AddVariable(const char* name, VariableType type, void* data);//declare and define a variable
	void AddVariableArray(const char* name, VariableType type, uint32_t count, void* data);// declare and define an array
	void AddDefinitionToVariable(const char* name, void* data);
	//Function controls
	void OpenFunction(const char* name);
	void AddReturnVariable(VariableType type);
	void AddInputVariable(const char* name, VariableType type);
	void CloseFunction();

	Variable& operator()(const char* name, VariableType type) {
		//for now just loop over all variables
		for (auto& v : m_CurrentScope->LocalVariables) {
			if (v.Name == name)
				return v;
		}
		for (auto& v : m_CurrentScope->InputVariables) {
			if (v.Name == name)
				return v;
		}
		Variable& v = m_CurrentScope->LocalVariables.push_back();
		v.Type = type;
		v.Array = false;
		v.ArrayCount = 1;
		v.Name = name;
		v.Data = AllocateVariable(VariableTypeSize[type]);
		return v;
	}

private:
	void* AllocateVariable(uint32_t size);

	eastl::vector<Variable> m_Variables;
	eastl::vector<eastl::vector<uint8_t>> m_VariableData;
	eastl::vector<Scope> m_Scopes;
	Scope* m_GlobalScope;
	eastl::vector<uint8_t>* m_CurrrentData;
	Scope* m_CurrentScope;
};