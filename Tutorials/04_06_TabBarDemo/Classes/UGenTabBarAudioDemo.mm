//
//  TabBarDemoAppDelegate.m
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "UGenTabBarAudioDemo.h"


@implementation UGenTabBarAudioDemo

@synthesize freq, amp;


// you can override init to initialise any non-UGen++ objects e.g., simple variables
// this should be called before initAudio
-(id)init
{
    if(self = [super init])
    {
        freq = 1000;
        amp = 0.1;
        
        [[NSNotificationCenter defaultCenter] postNotificationName:@"update" 
                                                            object:self];
    }
    
    return self;
}

// you can override initAudio to initialise any UGen++ objects e.g., UGens
// this will be called after init
-(id)initAudio
{
    // custom initialisation
    freqControl = Lag::AR(&freq, 0.1);
    ampControl = Lag::AR(&amp, 0.1);
    
    // always do this last in here since it calls constructGraph:
    return [super initAudio];
}


- (UGen)constructGraph:(UGen)input
{	
	return SinOsc::AR(freqControl, 0, ampControl);
}


@end