#ifndef XPM_SCRIPTING_H_
#define XPM_SCRIPTING_H_


enum class ScriptState
{
	NotReady = 0,
	Ready,			// also the Stopped state
	Running
};


class ScriptModule;

typedef std::vector<ScriptModule *>	vecScriptMods;

// Scripting Core class
class ScriptCore
{
private:
	vecScriptMods	mModules;		// registered script modules
	ScriptState		mState;			// current control state
	
public:
	string			file;			// requested script file to load


	ScriptCore();
	~ScriptCore();

	bool prepare();
	void start();
	void stop();
};

extern ScriptCore scripting;



// Scripting Module class
class ScriptModule
{
protected:
	volatile ScriptState	mState;			// current state

	
	ScriptModule();
	virtual ~ScriptModule();

	
public:
	virtual bool prepare()	= 0;
	virtual void start()	= 0;
	virtual void stop()		= 0;
};


#endif // XPM_SCRIPTING_H_
