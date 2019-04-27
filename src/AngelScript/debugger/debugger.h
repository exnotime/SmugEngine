#ifndef DEBUGGER_H
#define DEBUGGER_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <map>

BEGIN_AS_NAMESPACE

class CDebugger {
  public:
	CDebugger();
	virtual ~CDebugger();

	// Register callbacks to handle to-string conversions of application types
	// The expandMembersLevel is a counter for how many recursive levels the members should be expanded.
	// If the object that is being converted to a string has members of its own the callback should call
	// the debugger's ToString passing in expandMembersLevel - 1.
	typedef eastl::string (*ToStringCallback)(void *obj, int expandMembersLevel, CDebugger *dbg);
	virtual void RegisterToStringCallback(const asITypeInfo *ti, ToStringCallback callback);

	// User interaction
	virtual void TakeCommands(asIScriptContext *ctx);
	virtual void Output(const eastl::string &str);

	// Line callback invoked by context
	virtual void LineCallback(asIScriptContext *ctx);

	// Commands
	virtual void PrintHelp();
	virtual void AddFileBreakPoint(const eastl::string &file, int lineNbr);
	virtual void AddFuncBreakPoint(const eastl::string &func);
	virtual void ListBreakPoints();
	virtual void ListLocalVariables(asIScriptContext *ctx);
	virtual void ListGlobalVariables(asIScriptContext *ctx);
	virtual void ListMemberProperties(asIScriptContext *ctx);
	virtual void ListStatistics(asIScriptContext *ctx);
	virtual void PrintCallstack(asIScriptContext *ctx);
	virtual void PrintValue(const eastl::string &expr, asIScriptContext *ctx);

	// Helpers
	virtual bool InterpretCommand(const eastl::string &cmd, asIScriptContext *ctx);
	virtual bool CheckBreakPoint(asIScriptContext *ctx);
	virtual eastl::string ToString(void *value, asUINT typeId, int expandMembersLevel, asIScriptEngine *engine);

	// Optionally set the engine pointer in the debugger so it can be retrieved
	// by callbacks that need it. This will hold a reference to the engine.
	virtual void SetEngine(asIScriptEngine *engine);
	virtual asIScriptEngine *GetEngine();

  protected:
	enum DebugAction {
		CONTINUE,  // continue until next break point
		STEP_INTO, // stop at next instruction
		STEP_OVER, // stop at next instruction, skipping called functions
		STEP_OUT   // run until returning from current function
	};
	DebugAction        m_action;
	asUINT             m_lastCommandAtStackLevel;
	asIScriptFunction *m_lastFunction;

	struct BreakPoint {
		BreakPoint(eastl::string f, int n, bool _func) : name(f), lineNbr(n), func(_func), needsAdjusting(true) {}
		eastl::string name;
		int         lineNbr;
		bool        func;
		bool        needsAdjusting;
	};
	eastl::vector<BreakPoint> m_breakPoints;

	asIScriptEngine *m_engine;

	// Registered callbacks for converting types to strings
	std::map<const asITypeInfo*, ToStringCallback> m_toStringCallbacks;
};

END_AS_NAMESPACE

#endif