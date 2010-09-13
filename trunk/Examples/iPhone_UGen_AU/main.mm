#import <UIKit/UIKit.h>
#include "../../UGen/UGen.h"



// host class - must implement constructGraph
@interface UGenIPhoneDemoHost : UIKitAUIOHost
{ 
} 
@end

// view controller - the host class could go in here if you have only one view controller
@interface UGenIPhoneDemo : UIViewController
{	
	UIView *contentView;
}
@end

// app delegate
@interface UGenAppDelegate : NSObject <UIApplicationDelegate> //UIKitAUIOHost
{ 
	UGenIPhoneDemoHost* host;
} 
@property (nonatomic, retain) UGenIPhoneDemoHost* host;
@end 



@implementation UGenIPhoneDemoHost

- (id)init
{
	if (self = [super init])
	{
		
	}
	return self;
}

- (UGen)constructGraph:(UGen)input
{
	return SinOsc::AR(1000, 0, 0.1);
}

@end



@implementation UGenIPhoneDemo

- (id)init
{
	if (self = [super init])
	{
		self.title = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];		
	}
	return self;
}

- (void)loadView
{		
	contentView = [[UIView alloc] initWithFrame: [[UIScreen mainScreen] applicationFrame]];
	
	//... and subviews e.g., sliders etc
		
	// Provide support for auto-rotation and resizing
	contentView.autoresizesSubviews = YES;
	contentView.autoresizingMask = (UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight);

	// Assign the view to the view controller
	self.view = contentView;
    [contentView release];	
}

// Allow the view to respond to iPhone Orientation changes
-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

-(void) dealloc
{	
	// add any further clean-up here
	[contentView release];
	[super dealloc];
}

@end




@implementation UGenAppDelegate 

@synthesize host;

- (void)applicationDidFinishLaunching:(UIApplication *)application 
{ 
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]; 
	UGenIPhoneDemo *root = [[UGenIPhoneDemo alloc] init];
	UINavigationController *nav = [[UINavigationController alloc] initWithRootViewController:root]; 
	[window addSubview:nav.view]; 
	[window makeKeyAndVisible]; 
		
	// initialise the audio host class and the audio (audio could be initialised later if required)
	host = [[[UGenIPhoneDemoHost alloc] init] initAudio];
} 

- (void)applicationWillTerminate:(UIApplication *)application 
{ 
	// shutdown the audio host, this function will not return until a brief 
	// fade-out has been performed
	[host shutdown]; 
} 

- (void)dealloc 
{ 
	[super dealloc]; 
} 

@end 

int main(int argc, char *argv[]) 
{ 
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init]; 
	int retVal = UIApplicationMain(argc, argv, nil, @"UGenAppDelegate"); 
	[pool release]; 
	return retVal; 
};
