#include "PythonModule.h"

// Stringify and concatenate the PYBIND11_VERSION_xxx preprocessor symbols
#define Str(x) #x
#define QUOTE(x) Str(x)
#define PYBIND11_VERSION_STR (QUOTE(PYBIND11_VERSION_MAJOR) "." QUOTE(PYBIND11_VERSION_MINOR) "." QUOTE(PYBIND11_VERSION_PATCH))

#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>
#include <pybind11/attr.h>
#include <pybind11/eval.h>

#include "PythonConsoleWriter.h"
#include "itextstream.h"
#include "os/file.h"
#include "os/path.h"

namespace script
{

namespace
{
    constexpr const char* ModuleName = "darkradiant";
}

PythonModule::PythonModule() :
    _outputWriter(false, _outputBuffer),
    _errorWriter(true, _errorBuffer),
    _interpreterInitialised(false)
{
    registerModule();
}

PythonModule::~PythonModule()
{
    _namedInterfaces.clear();

    for (auto& module : _builtInModules)
    {
        module.dec_ref();
        module.release();
    }

    _builtInModules.clear();

    // Release the references to trigger the internal cleanup before Py_Finalize
    _module.dec_ref();
    _module.release();

    _globals.reset();

    py::finalize_interpreter();

    // The memory allocated to initialise the module_def structure
    // needs to stay alive until the interpreter is finalised
    _moduleDef.reset();
}

void PythonModule::initialise()
{
    py::initialize_interpreter();

    try
    {
        // Import the darkradiant module
        // This is triggering the call to InitModule as passed to the inittab
        // so we need to set the static _instance pointer right before and clear it afterwards
        _instance = this;

        py::module::import(ModuleName);

        // Construct the console writer interface
        PythonConsoleWriterClass consoleWriter(_module, "PythonConsoleWriter");
        consoleWriter.def(py::init<bool, std::string&>());
        consoleWriter.def("write", &PythonConsoleWriter::write);
        consoleWriter.def("flush", &PythonConsoleWriter::flush);

        // Redirect stdio output to our local ConsoleWriter instances
        py::module::import("sys").attr("stderr") = &_errorWriter;
        py::module::import("sys").attr("stdout") = &_outputWriter;

        // String vector is used in multiple places
        py::bind_vector< std::vector<std::string> >(_module, "StringVector");
    }
    catch (const py::error_already_set& ex)
    {
        rError() << ex.what() << std::endl;
    }

    // Not needed anymore
    _instance = nullptr;
}

void PythonModule::registerModule()
{
    rMessage() << "Registering darkradiant module to Python using pybind11 version " <<
        PYBIND11_VERSION_STR << std::endl;

    // Register the darkradiant module to Python, the InitModule function
    // will be called as soon as the module is imported
    int result = PyImport_AppendInittab(ModuleName, InitModule);

    if (result == -1)
    {
        rError() << "Could not initialise Python module" << std::endl;
        return;
    }
}

ExecutionResultPtr PythonModule::executeString(const std::string& scriptString)
{
    ExecutionResultPtr result = std::make_shared<ExecutionResult>();

    result->errorOccurred = false;

    // Clear the output buffers before starting to execute
    _outputBuffer.clear();
    _errorBuffer.clear();

    try
    {
        std::string fullScript = "import " + std::string(ModuleName) + " as DR\n"
            "from " + std::string(ModuleName) + " import *\n";
        fullScript.append(scriptString);

        // Attempt to run the specified script
        py::eval<py::eval_statements>(fullScript, getGlobals());
    }
    catch (py::error_already_set& ex)
    {
        _errorBuffer.append(ex.what());
        result->errorOccurred = true;

        rError() << "Error executing script: " << ex.what() << std::endl;
    }

    result->output += _outputBuffer + "\n";
    result->output += _errorBuffer + "\n";

    _outputBuffer.clear();
    _errorBuffer.clear();

    return result;
}

void PythonModule::executeScriptFile(const std::string& scriptBasePath, const std::string& relativeScriptPath, bool setExecuteCommandAttr)
{
    try
    {
        auto fullPath = scriptBasePath + relativeScriptPath;

        // Prevent calling exec_file with a non-existent file, we would
        // get crashes during Py_Finalize later on.
        if (!os::fileOrDirExists(fullPath))
        {
            rError() << "Error: File " << fullPath << " doesn't exist." << std::endl;
            return;
        }

        py::dict locals;

        if (setExecuteCommandAttr)
        {
            locals["__executeCommand__"] = true;
        }

        // Attempt to run the specified script
        py::eval_file(fullPath, getGlobals(), locals);
    }
    catch (std::invalid_argument& e)
    {
        rError() << "Error trying to execute file " << relativeScriptPath << ": " << e.what() << std::endl;
    }
    catch (const py::error_already_set& ex)
    {
        rError() << "Error while executing file: " << relativeScriptPath << ": " << std::endl;
        rError() << ex.what() << std::endl;
    }
}

py::dict& PythonModule::getGlobals()
{
    if (!_globals)
    {
        _globals.reset(new py::dict);
    }

    return *_globals;
}

#if PY_MAJOR_VERSION >= 3
PyObject* PythonModule::InitModule()
{
    if (!_instance) throw new std::runtime_error("_instance reference not set");
	return _instance->initialiseModule();
}
#else
void PythonModule::InitModule()
{
    if (!_instance) throw new std::runtime_error("_instance reference not set");
    _instance->initialiseModule();
}
#endif

PyObject* PythonModule::initialiseModule()
{
	try
	{
        _moduleDef = std::make_unique<py::module::module_def>();
        _module = py::module::create_extension_module(ModuleName, "DarkRadiant Main Module", _moduleDef.get());

        // Add the registered interfaces
        for (const auto& i : _namedInterfaces)
        {
            // Handle each interface in its own try/catch block
            try
            {
                i.second->registerInterface(_module, getGlobals());
            }
            catch (const py::error_already_set& ex)
            {
                rError() << "Error while initialising interface " << i.first << ": " << std::endl;
                rError() << ex.what() << std::endl;
            }
        }

		auto main = py::module::import("__main__");
		auto globals = main.attr("__dict__").cast<py::dict>();

		for (auto i = globals.begin(); i != globals.end(); ++i)
		{
            getGlobals()[(*i).first] = (*i).second;
		}

        _interpreterInitialised = true;

		return _module.ptr();
	}
	catch (py::error_already_set& e)
	{
		//e.clear();
		PyErr_SetString(PyExc_ImportError, e.what());
		return nullptr;
	}
	catch (const std::exception& e)
	{
		PyErr_SetString(PyExc_ImportError, e.what());
		return nullptr;
	}
}

void PythonModule::addInterface(const NamedInterface& iface)
{
    // Check if exists
    if (interfaceExists(iface.first))
    {
        rError() << "A script interface with the name " << iface.first << " is already registered." << std::endl;
        return;
    }

    // Add to the list to hold the reference
    _namedInterfaces.emplace_back(iface);

    // Initialise the interface at once, if the module is already alive
    if (_interpreterInitialised)
    {
        iface.second->registerInterface(_module, getGlobals());
    }
}

bool PythonModule::interfaceExists(const std::string& name)
{
    // Traverse the interface list
    for (const auto& namedInterface : _namedInterfaces)
    {
        if (namedInterface.first == name)
        {
            return true;
        }
    }

    return false;
}

ScriptCommand::Ptr PythonModule::createScriptCommand(const std::string& scriptBasePath, const std::string& relativeScriptPath)
{
    try
    {
        auto fullPath = scriptBasePath + relativeScriptPath;

        // Create a new dictionary for the initialisation routine
        py::dict locals;

        // Disable the flag for initialisation, just to be sure
        locals["__executeCommand__"] = false;

        // Attempt to run the specified script
        py::eval_file(fullPath, getGlobals(), locals);

        std::string cmdName;
        std::string cmdDisplayName;

        if (locals.contains("__commandName__"))
        {
            cmdName = locals["__commandName__"].cast<std::string>();
        }

        if (locals.contains("__commandDisplayName__"))
        {
            cmdDisplayName = locals["__commandDisplayName__"].cast<std::string>();
        }

        if (!cmdName.empty())
        {
            if (cmdDisplayName.empty())
            {
                cmdDisplayName = cmdName;
            }

            // Successfully retrieved the command
            return std::make_shared<ScriptCommand>(cmdName, cmdDisplayName, relativeScriptPath);
        }

        rError() << "Script file " << relativeScriptPath << " does not export a __commandName__ value" << std::endl;
        return {};
    }
    catch (const py::error_already_set& ex)
    {
        rError() << "Script file " << relativeScriptPath << " is not a valid command:" << std::endl;
        rError() << ex.what() << std::endl;
        return {};
    }
}

void PythonModule::registerBuiltInModulePath(const std::string& scriptBasePath)
{
    // Import module from the base path
    auto sys = py::module::import("sys");
    sys.attr("path").attr("insert")(1, os::getDirectory(scriptBasePath));
}

void PythonModule::initialiseBuiltInModule(const std::string& moduleFilename)
{
    try
    {
        auto moduleName = os::removeExtension(os::getFilename(moduleFilename));
        auto builtInModule = py::module::import(moduleName.c_str());
        _builtInModules.push_back(builtInModule);
    }
    catch (const py::error_already_set& ex)
    {
        rError() << "Script file " << moduleFilename << " is not a valid command:" << std::endl;
        rError() << ex.what() << std::endl;
    }
}

void PythonModule::refreshBuiltInModules()
{
    for (auto& module : _builtInModules)
    {
        module.reload();
    }
}

PythonModule* PythonModule::_instance = nullptr;

}
