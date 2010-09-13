
//#ifdef WIN_VERSION
	#define _CRT_SECURE_NO_WARNINGS 1 // don't warn us about sprintf being insecure in Windows!	

	/*
		if you're getting an error here on Windows you need to set up your compiler
		paths to the c74support/max-includes directory

	*/
//#ifdef WIN_VERSION
//	#include "common\dllmain_win.c"					// includes ext.h
//#else
	#include "ext.h"
//#endif

// Max
#include "ext_obex.h"
#include "jpatcher_api.h"
#include "jgraphics.h"


// Max: not yet in the .app frameworks on Mac
//#include "ext_drag.h"
//#include "ext_time.h"
//#include "ext_itm.h"
//#include "ext_atomic.h"


#define MAXOBJECT_NAME PROJECT_NAME  // PROJECT_NAME is defined in the project settings preprocessor macros

/* // win script?
echo LIBRARY $(TargetFileName) > "$(IntDir)\$(TargetName).def"
echo EXPORTS >> "$(IntDir)\$(TargetName).def"
echo main >> "$(IntDir)\$(TargetName).def"
*/