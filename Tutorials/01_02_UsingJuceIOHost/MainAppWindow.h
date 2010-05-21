#ifndef _MainAppWindow_H__
#define _MainAppWindow_H__

/*
 Using <juce/juce.h> here assumes the juce source tree is accessible
 
 In Xcode add the directory containing 'juce' as a source tree named JUCE_SOURCE
 */
#include <juce/juce.h>

#define VALIDCHARS "ABCDEFGHIJKLMNOPQRTSUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"
#define PROJECT_NAME_IDENTIFIER (String(PROJECT_NAME).replaceCharacter (' ', '_')		\
													 .replaceCharacter ('.', '_')		\
													 .retainCharacters (T(VALIDCHARS)))

class MainAppWindow  : public DocumentWindow
{
public:
	MainAppWindow();
	~MainAppWindow();
	
	void closeButtonPressed();
	
};


#endif 