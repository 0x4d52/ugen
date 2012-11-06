//
//  FirstViewController.m
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SecondViewController.h"
#import "TabBarDemoAppDelegate.h"
#import "UGenTabBarAudioDemo.h"

@implementation SecondViewController


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    [[NSNotificationCenter defaultCenter] addObserver:self 
                                             selector:@selector(updateNotify) 
                                                 name:@"update" 
                                               object:nil];
}    

- (void)viewWillAppear:(BOOL)animated
{
    [self updateParameters];
}

-(void)updateNotify
{
    [self performSelectorOnMainThread:@selector(updateParameters) 
                           withObject:nil 
                        waitUntilDone:false];
}

-(void)updateParameters
{
    // update the view from the audio settings
    TabBarDemoAppDelegate* app = (TabBarDemoAppDelegate*)[[UIApplication sharedApplication] delegate];
    UGenTabBarAudioDemo* audio = app.audio;
    ampSlider.value = audio.amp;
}

- (void)viewDidUnload {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super viewDidUnload];
}


- (void)dealloc {
    [super dealloc];
}

@end
