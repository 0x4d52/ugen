//
//  TabBarDemoAppDelegate.m
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "TabBarDemoAppDelegate.h"
#import "UGenTabBarAudioDemo.h"

@implementation TabBarDemoAppDelegate

@synthesize window;
@synthesize tabBarController;
@synthesize audio;


- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    // Add the tab bar controller's current view as a subview of the window
    [window addSubview:tabBarController.view];
	
	audio = [[UGenTabBarAudioDemo alloc] init];
	[audio setPreferredBufferSize: 2048];
	[audio setPreferredSampleRate: 22050];
	[audio initAudio];
}

- (void)applicationWillTerminate:(UIApplication *)application 
{ 
	[audio shutdown]; 
} 

-(IBAction)freqChanged:(UISlider*)slider
{
	audio.freq = [slider value];
}

-(IBAction)ampChanged:(UISlider*)slider
{
	audio.amp = [slider value];
}


/*
// Optional UITabBarControllerDelegate method
- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController {
}
*/

/*
// Optional UITabBarControllerDelegate method
- (void)tabBarController:(UITabBarController *)tabBarController didEndCustomizingViewControllers:(NSArray *)viewControllers changed:(BOOL)changed {
}
*/


- (void)dealloc {
    [tabBarController release];
    [window release];
    [super dealloc];
}

@end

