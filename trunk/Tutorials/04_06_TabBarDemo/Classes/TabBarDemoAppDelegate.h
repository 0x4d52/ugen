//
//  TabBarDemoAppDelegate.h
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class UGenTabBarAudioDemo;

@interface TabBarDemoAppDelegate : NSObject <UIApplicationDelegate, UITabBarControllerDelegate> 
{
    UIWindow *window;
    UITabBarController *tabBarController;
	UGenTabBarAudioDemo* audio;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UITabBarController *tabBarController;
@property (nonatomic, readonly) UGenTabBarAudioDemo* audio;


-(IBAction)freqChanged:(UISlider*)slider;
-(IBAction)ampChanged:(UISlider*)slider;


@end
