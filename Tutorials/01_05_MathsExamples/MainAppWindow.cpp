#include "MainAppWindow.h"
#include "MainComponent.h"

//==============================================================================
MainAppWindow::MainAppWindow()
:   
DocumentWindow (
				T(PROJECT_NAME),				// Set the text to use for the title
				Colours::azure,					// Set the colour of the window
				DocumentWindow::allButtons,		// Set which buttons are displayed
				true							// This window should be added to the desktop
				)
{
    setResizable (true, false); 
	setTitleBarHeight (22);     
	
    MainComponent* contentComponent = new MainComponent ();
	
    setContentOwned (contentComponent, false);
}

MainAppWindow::~MainAppWindow()
{
}

void MainAppWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
} 