#ifndef XPM_SCRIPTING_PYTHON_H_
#define XPM_SCRIPTING_PYTHON_H_



// Python script module class
class ScriptPython : public ScriptModule
{
protected:

	volatile bool mRun;

	void thread();
	
public:
	ScriptPython();
	virtual ~ScriptPython();

	virtual bool prepare();
	virtual void start();
	virtual void stop();
};


#endif // XPM_SCRIPTING_PYTHON_H_
