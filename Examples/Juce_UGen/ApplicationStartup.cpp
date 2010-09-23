/*
 ==============================================================================
 
 JUCE library : Starting point code, v1.26
 Copyright 2005 by Julian Storer. [edited by haydxn, 3rd April 2007]
 
 ------------------------------------------------------------------------------
 
 ApplicationStartup.cpp :
 
 This file describes how the application will be brought to life within the
 operating system. The basic order of things is...
 
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
 The [OS] creates the 'AppClass', which is a shell for the program,
 and is responsible for bringing everything to life...
 
 ... the [AppClass] creates the MainAppWindow and puts it on the screen...
 
 ... the [MainAppWindow] is a visible base for the program, and it
 creates the program's MainComponent upon itself...
 
 ... the [MainComponent] then 'does' the main 'program stuff'
 
 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 
 If you're only writing simple applications, there's very little that you'd need
 to do in here, as the app shell is only really responsible for creating the window
 and getting everything running.
 
 You will, however, probably want to set the size of the window to something suitable.
 This is done in the initialise function, after the window has been created.
 
 ------------------------------------------------------------------------------
 
 Please feel free to do whatever you like with this code, bearing in mind that
 it's not guaranteed to be bug-free!
 
 ==============================================================================
 */

/* 
 If you get an error here make sure the directory which contains 'juce' is
 in the search path. In the Mac project this directory should be assigned
 to JUCE_SOURCE in the Xcode -> Preferences.. -> Source Trees (just make
 "JUCE_SOURCE" the setting name and display name).
 */
#include <juce/juce.h>
#include "../../UGen/UGen.h"
#include "MainAppWindow.h"


//==============================================================================
class AppClass : public JUCEApplication
	{
		/* Important! NEVER embed objects directly inside your JUCEApplication class! Use
		 ONLY pointers to objects, which you should create during the initialise() method
		 (NOT in the constructor!) and delete in the shutdown() method (NOT in the
		 destructor!)
		 
		 This is because the application object gets created before Juce has been properly
		 initialised, so any embedded objects would also get constructed too soon.
		 */
		MainAppWindow* theMainWindow;
		
	public:
		//==============================================================================
		AppClass()
        : theMainWindow (0)
		{
			// This is where the application itself is created. It's a bit like a shell, which
			// all your real app stuff gets born into. At this point (where the app pops into
			// being), Juce doesn't really 'exist' yet, so we can't do anything with it here.
			// Once this application shell has been established, Juce will be awake so we can
			// create instances of its classes willy-nilly.
			
			// [Jules says...]
			// NEVER do anything in here that could involve any Juce function being called
			// - leave all your startup tasks until the initialise() method.
			
		}
		
		~AppClass()
		{
			// This is where the application body is destroyed, and making any Juce calls in
			// here could be very dangerous.
			
			// Just as Juce wasn't awake when the shell was created, by this stage it's been
			// tidied away and put to sleep. This is the bit that happens AFTER your program
			// code has ended (and hopefully put all its toys away).
			
			// [Jules says...]
			// Your shutdown() method should already have done all the things necessary to
			// clean up this app object, so you should never need to put anything in
			// the destructor.
		}
		
		//==============================================================================
		void initialise (const String& commandLine)
		{
			UGen::initialise();			
			ugen_reportObjectSizes();
			
			// This is called automatically when the application is ready to launch.
			// So far, it just exists in memory as an empty pocket of potential waiting
			// to burst into life as a program. Nothing yet exists to act or be displayed.
			
			// All we want to do here is create the main window. This instantiates an object
			// of 'MainAppWindow' - which we have defined in MainAppWindow(.h/.cpp). The app's
			// behaviour comes from that, so all we need is to bring it to life...
			theMainWindow = new MainAppWindow();
			// ... and plonk it onto the display...
			theMainWindow->centreWithSize (800, 600);   // [*] (see below for a tip on this)
			// ... (of course making sure that it is visible!)
			theMainWindow->setVisible (true);
			
			// That's all we have to do here. Once this function has ended, the Juce
			// application will start firing its event loop. This is basically the
			// engine that powers the Juce app classes (giving life to the Component
			// and messaging model), and is something that will just happen by itself.
			
			// The event dispatch loop will keep the app alive until something calls
			// JUCEApplication::quit() - which could be windows closing the application,
			// or the user clicking the window's close button.
			
			// [*] When you set the size of the window, there is something important to bear
			//     in mind: The dimensions you set here are for the window component itself.
			//     If this is a DocumentWindow (which is the most obvious thing for it to be),
			//     then it will have a titlebar and a border. The content component sits inside
			//     this arrangement of parts - which means that your program's main component
			//     dimensions will actually be slightly smaller than the values you set here.
			//     This only matters if you're relying on positioning your program's various
			//     widgets using constant values instead of calculating relative positions from
			//     the available dimensions (i.e. saying 'this button is 50 pixels wide' instead
			//     of 'this button is 1/3 of the width of its parent component').
		}
		
		void shutdown()
		{
			// This gets called when the application is ready to shut down.
			// Anything that we created in the initialise() function should be destroyed,
			// so that nothing is left hanging around when the app shell ceases to exist.
			
			// All we need to do here is delete the MainAppWindow we created...
			deleteAndZero (theMainWindow);
			
			//Time::waitForMillisecondCounter(Time::getMillisecondCounter()+ 500);
			UGen::shutdown();
		}
		
		//==============================================================================
		const String getApplicationName()
		{
			// The name for the application
			return T("Juce tutorial application");
		}
		
		const String getApplicationVersion()
		{
			// Here we can give a 'version' indicator, to distinguish one build from
			// another. If you update your program, it's a good idea to also update
			// the string returned here.
			return T("0.0");
		}
		
		bool moreThanOneInstanceAllowed()
		{
			// We can prevent multiple instances of the application here by returning false.
			return true;
		}
		
		void anotherInstanceStarted (const String& commandLine)
		{
			// This will get called if the user launches another copy of the application.
		}
	};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION(AppClass) 