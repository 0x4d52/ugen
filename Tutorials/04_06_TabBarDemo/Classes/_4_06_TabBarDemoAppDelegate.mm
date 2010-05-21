//
//  _4_06_TabBarDemoAppDelegate.m
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "_4_06_TabBarDemoAppDelegate.h"


@implementation UGenTabBarAudioDemo

@synthesize freq, amp;

- (UGen)constructGraph:(UGen)input
{
	freq = 1000;
	amp = 0.1;
	
	UGen freqControl = Lag::AR(&freq, 0.1);
	UGen ampControl = Lag::AR(&amp, 0.1);
	
	return SinOsc::AR(freqControl, 0, ampControl);
}

@end



@implementation _4_06_TabBarDemoAppDelegate

@synthesize window;
@synthesize tabBarController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    // Add the tab bar controller's current view as a subview of the window
    [window addSubview:tabBarController.view];
	
	audio = [[[UGenTabBarAudioDemo alloc] init] initAudio];
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

