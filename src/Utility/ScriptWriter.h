#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

//Built in variable types
enum VariableType {
	VAR_VOID,
	VAR_INT,
	VAR_BOOL,
	VAR_FLOAT,
	VAR_VEC2,
	VAR_VEC3,
	VAR_VEC4,
	VAR_QUAT,
	VAR_MAT3,
	VAR_MAT4,
	VARIABLE_TYPE_COUNT
};

//Must match the size of the enum VariableType
static uint32_t VariableTypeSize[VARIABLE_TYPE_COUNT] = {
	0,
	sizeof(int),
	sizeof(bool),
	sizeof(float),
	sizeof(glm::vec2),
	sizeof(glm::vec3),
	sizeof(glm::vec4),
	sizeof(glm::vec4), //quaternion is a vec4
	sizeof(glm::mat3),
	sizeof(glm::mat4)
};

static const char* VariableTypeNames[VARIABLE_TYPE_COUNT] = {
	"void",
	"int",
	"bool",
	"float",
	"vec2",
	"vec3",
	"vec4",
	"quat",
	"mat3",
	"mat4"
};

struct Variable {
	std::string Name;
	VariableType Type;
	void* Data;
	bool Array;
	uint32_t ArrayCount;
};

struct Scope {
	std::string Name;
	Variable ReturnVariable;
	std::vector<Variable> InputVariables;
	std::vector<Variable> LocalVariables;
	std::string Code;
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
	//Function controls
	void OpenFunction(const char* name);
	void AddReturnVariable(VariableType type);
	void AddInputVariable(const char* name, VariableType type);
	void CloseFunction();
private:
	void* AllocateVariable(uint32_t size);

	std::vector<Variable> m_Variables;
	std::vector<uint8_t> m_VariableData;
	std::vector<Scope> m_Scopes;
	Scope* m_GlobalScope;

	Scope* m_CurrentScope;
};