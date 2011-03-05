// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-10 by Martin Robinson www.miajo.co.uk
 
 ------------------------------------------------------------------------------
 
 UGEN++ can be redistributed and/or modified under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.
 
 UGEN++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with UGEN++; if not, visit www.gnu.org/licenses or write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA
 
 The idea for this project and code in the UGen implementations is
 devived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 ==============================================================================
 */

#include "../core/ugen_StandardHeader.h"

#ifdef UGEN_IPHONE

BEGIN_UGEN_NAMESPACE

#include "ugen_NSUtilities.h"

Text NSUtilities::pathInDirectory(Locations location, const char *filename) throw()
{
	ugen_assert(filename != 0);
	
	switch(location)
	{
		case Bundle: {
			NSString *nsFilename = [[NSString alloc] initWithUTF8String: filename];
			NSString* nspath = [[NSBundle mainBundle] pathForResource:nsFilename ofType:nil];
			[nsFilename release];
			
			if(!nspath)
				return "";
			else
				return [nspath UTF8String];
			
		} break;
			
		case Documents: {
			NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
			NSString *documentsDirectory = [paths objectAtIndex:0];
			if (!documentsDirectory) 
				return "";
			else
			{
				NSString *nsFilename = [[NSString alloc] initWithUTF8String: filename];
				NSString *appFile = [documentsDirectory stringByAppendingPathComponent:nsFilename];
				[nsFilename release];
				return [appFile UTF8String];
			}
			
		} break;
		
		case Temporary: {
//			NSString *tmp = NSTemporaryDirectory();
//			if(!tmp)
//				return "";
//			else
//			{
//				NSString *nsFilename = [[NSString alloc] initWithUTF8String: filename];
//				NSString *appFile = [tmp stringByAppendingPathComponent:nsFilename];
//				[nsFilename release];
//				return [appFile UTF8String];
//			}
			
			NSString *home = NSHomeDirectory();
			if(!home)
				return "";
			else
			{
				return Text([home UTF8String]) << Text("/tmp/") << Text(filename);
			}
			
		}
			
		default:
			return "";
	}
	
}

Text NSUtilities::stringToText(CFStringRef string) throw()
{
	return Text([(NSString*)string UTF8String]);
}


CFStringRef NSUtilities::textToString(Text const& text) throw()
{
	return (CFStringRef)[NSString stringWithUTF8String: text.getArray()];
}

END_UGEN_NAMESPACE

#endif // UGEN_IPHONE
