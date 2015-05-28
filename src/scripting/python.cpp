#include <xpmcommon.h>
#include "scripting/scripting.h"
#include "scripting/python.h"

// warnings workaround before including python header
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE
#include <Python.h>
#include <structmember.h>

#include "scripting/python_matrix.h"


//=============================================================================
// Python script module class
//=============================================================================

ScriptPython::ScriptPython()
 :	ScriptModule(),
	mRun(false)
{
	// initialize python
	Py_Initialize();
}
ScriptPython::~ScriptPython()
{
	// make sure we're stopped
	stop();
	
	// shutdown python
	Py_Finalize();
}


//-----------------------------------------------------------------------------
// Python script control functions
//-----------------------------------------------------------------------------

bool ScriptPython::prepare()
{
//	Py_InitThreads();	??


	if(!PythonHook_Matrix())
		return false;

	// success
	mState = ScriptState::Ready;
	return true;
}

void ScriptPython::start()
{
	if(mState != ScriptState::Ready)
		return;

	string file = "./scripts/main.py";
	string mod  = "main";

	if(!scripting.file.empty())
	{
		file = "./scripts/" + scripting.file;

		size_t pos = scripting.file.find('.');
		if(pos != string::npos)
			mod = scripting.file.substr(0, pos);
		else
			mod = scripting.file;
	}

	mod = "import " + mod + "\n";
	
	
//	char *argv[] = { (char *)"./scripts/main.py" };
	char *argv[] = { (char *)file.c_str() };
	PySys_SetArgv(1, argv);

	mState = ScriptState::Running;
//	PyRun_SimpleString("import main\n");
	PyRun_SimpleString(mod.c_str());
	mState = ScriptState::Ready;
}

void ScriptPython::stop()
{
	if(mState != ScriptState::Running)
		return;

	// ..
//	PyErr_SetInterrupt();

	mState = ScriptState::Ready;
}


//-----------------------------------------------------------------------------
// Working thread
//-----------------------------------------------------------------------------

void ScriptPython::thread()
{
	
}


