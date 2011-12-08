//
//  SecondViewController.h
//  04_06_TabBarDemo
//
//  Created by Martin Robinson on 28/01/2010.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface SecondViewController : UIViewController {
    IBOutlet UISlider* ampSlider;
}

-(void)updateParameters;

@end
