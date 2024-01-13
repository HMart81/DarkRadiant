#pragma once

#include "imodule.h"
#include <sigc++/signal.h>

namespace script
{

struct ExecutionResult
{
	// The output of the script
	std::string	output;

	// whether an error occurred
	bool	errorOccurred;
};
typedef std::shared_ptr<ExecutionResult> ExecutionResultPtr;

// Declared in iscriptinterface.h
class IScriptInterface;
typedef std::shared_ptr<IScriptInterface> IScriptInterfacePtr;

// Represents a named, executable .py script file
class IScriptCommand
{
public:
	virtual ~IScriptCommand() {}

	virtual const std::string& getName() const = 0;
	virtual const std::string& getFilename() const = 0;
	virtual const std::string& getDisplayName() const = 0;
};

/**
 * DarkRadiant's Scripting System, based on pybind11. It's possible
 * to expose additional interfaces by using the addInterface() method.
 */
class IScriptingSystem :
	public RegisterableModule
{
public:
	/**
	 * greebo: Add a named interface to the scripting system. The interface object
	 * must provide a "registerInterface" method which will declare the names
	 * and objects to the given namespace.
	 */
	virtual void addInterface(const std::string& name, const IScriptInterfacePtr& iface) = 0;

	/**
	 * greebo: Executes the given python script file. The filename is specified relatively
	 * to the scripts/ folder.
	 */
	virtual void executeScriptFile(const std::string& filename) = 0;

	/**
	 * greebo: Interprets the given string as python script.
	 *
	 * @returns: the result object.
	 */
	virtual script::ExecutionResultPtr executeString(const std::string& scriptString) = 0;

	/**
	 * Iterate over all available script commands, invoking the given functor.
	 */
	virtual void foreachScriptCommand(const std::function<void(const IScriptCommand&)>& functor) = 0;

	/**
	 * Signal fired when the available set of scripts has been reloaded
	 */
	virtual sigc::signal<void()>& signal_onScriptsReloaded() = 0;
};
typedef std::shared_ptr<IScriptingSystem> IScriptingSystemPtr;

} // namespace script

// String identifier for the script module
const char* const MODULE_SCRIPTING_SYSTEM("ScriptingSystem");

// This is the accessor for the scripting system
inline script::IScriptingSystem& GlobalScriptingSystem()
{
	static module::InstanceReference<script::IScriptingSystem> _reference(MODULE_SCRIPTING_SYSTEM);
	return _reference;
}
