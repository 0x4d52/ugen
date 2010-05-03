//
//  _4_06_TabBarDemoAppDelegate.h
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
}

@property (nonatomic) float freq;
@property (nonatomic) float amp;

@end


@interface _4_06_TabBarDemoAppDelegate : NSObject <UIApplicationDelegate, UITabBarControllerDelegate> 
{
    UIWindow *window;
    UITabBarController *tabBarController;
	UGenTabBarAudioDemo* audio;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UITabBarController *tabBarController;

@end
