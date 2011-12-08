//
//  TabBarDemoAppDelegate.h
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "../../../UGen/UGen.h"

@interface UGenTabBarAudioDemo : UIKitAUIOHost
{
	float freq, amp;
    UGen freqControl, ampControl;
}

@property (nonatomic) float freq;
@property (nonatomic) float amp;

@end
