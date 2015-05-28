#include <xpmcommon.h>
#include "scripting/scripting.h"
#include "scripting/python.h"


//=============================================================================
// Scripting Core class
//=============================================================================

ScriptCore::ScriptCore()
 :	mState(ScriptState::NotReady)
{
}
ScriptCore::~ScriptCore()
{
}


//-----------------------------------------------------------------------------
// Scripting operation control functions
//-----------------------------------------------------------------------------

bool ScriptCore::prepare()
{
	vecScriptMods::iterator it;


	// register supported scripting modules
	mModules.push_back(new ScriptPython());


	// initialize scripting modules
	for(it = mModules.begin(); it != mModules.end(); ++it)
	{
		// fatal abort on initialization failure
		if(!(*it)->prepare())
			return false;
	}
	
	// success
	mState = ScriptState::Ready;
	return true;
}

void ScriptCore::start()
{
	if(mState != ScriptState::Ready)
		return;
	
	vecScriptMods::iterator it;

	// start all scripting modules
	for(it = mModules.begin(); it != mModules.end(); ++it)
		(*it)->start();

	mState = ScriptState::Running;
}

void ScriptCore::stop()
{
	if(mState != ScriptState::Running)
		return;
	
	vecScriptMods::iterator it;

	// stop all scripting modules
	for(it = mModules.begin(); it != mModules.end(); ++it)
		(*it)->stop();

	mState = ScriptState::Ready;
}



//=============================================================================
// Scripting Module class
//=============================================================================

ScriptModule::ScriptModule()
 :	mState(ScriptState::NotReady)
{
	
}
ScriptModule::~ScriptModule()
{
}


