#include "MainAppWindow.h"
#include "../../UGen/UGen.h"

//==============================================================================
class AppClass : public JUCEApplication
	{
		MainAppWindow* theMainWindow;
		
	public:
		AppClass() : theMainWindow (0) { }
		
		~AppClass()	{ }
		
		//==============================================================================
		void initialise (const String& commandLine)
		{			
			theMainWindow = new MainAppWindow();
			theMainWindow->centreWithSize (300, 300);
			theMainWindow->setVisible (true);
		}
		
		void shutdown()
		{
			deleteAndZero (theMainWindow);
		}
		
		//==============================================================================
		const String getApplicationName()
		{
			return T(PROJECT_NAME);
		}
		
		const String getApplicationVersion()
		{
			return T("0.0");
		}
		
		bool moreThanOneInstanceAllowed()
		{
			return true;
		}
		
		void anotherInstanceStarted (const String& commandLine)
		{
		}
	};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION(AppClass) 