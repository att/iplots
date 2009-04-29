//
//  CocoaApp.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaApp.h"
#import "CocoaWindow.h"
#import "AVector.h"
#import "AScatterPlot.h"

@implementation CocoaApp

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	float data_x[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
	float data_y[] = { 1.0, 2.0, 1.5, 3.0, 5.0, 6.0 };		
	AVector *vx = new AFloatVector(data_x, sizeof(data_x)/sizeof(data_x[0]));
	AVector *vy = new AFloatVector(data_y, sizeof(data_y)/sizeof(data_y[0]));
	ARect aFrame = AMkRect(0, 0, 400, 300);
	AVisual *visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	vx->release();
	vy->release();
	
	NSRect rect = NSMakeRect(100, 100, aFrame.width, aFrame.height);
	CocoaWindow *window = [[CocoaWindow alloc] initWithContentRect:rect visual:visual];
	visual->release();

    [window makeKeyAndOrderFront:nil];
	[window setDelegate:self];
	// no idea why, but the shadow is not there until you cycle the shadow setting...
	[window setHasShadow:NO];
	[window setHasShadow:YES];
}

@end
