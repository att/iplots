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
#import "REngine.h"

CocoaWindow *ACocoa_CreateWindow(AVisual *visual, APoint position)
{
	ARect aFrame = visual->frame();
	NSRect rect = NSMakeRect(position.x, position.y, aFrame.width + aFrame.x, aFrame.height + aFrame.y);

	CocoaWindow *window = [[CocoaWindow alloc] initWithContentRect:rect visual:visual];
	
    [window makeKeyAndOrderFront:nil];
	// [window setDelegate:self];
	// no idea why, but the shadow is not there until you cycle the shadow setting...
	[window setHasShadow:NO];
	[window setHasShadow:YES];
	return window;
}

@implementation CocoaApp

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
#if 0
	float data_x[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
	float data_y[] = { 1.0, 2.0, 1.5, 3.0, 5.0, 6.0 };
	AMarker *mark = new AMarker(sizeof(data_x)/sizeof(data_x[0]));
	ADataVector *vx = new AFloatVector(mark, data_x, sizeof(data_x)/sizeof(data_x[0]), true);
	ADataVector *vy = new AFloatVector(mark, data_y, sizeof(data_y)/sizeof(data_y[0]), true);
#else
	REngine *eng = REngine::mainEngine();
	RObject *o = eng->parseAndEval("rnorm(1e6)");
	AMarker *mark = new AMarker(o->length());
	ADataVector *vx = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
	o = eng->parseAndEval("rnorm(1e6)");	
	ADataVector *vy = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
#endif
	
	ARect aFrame = AMkRect(0, 0, 400, 300);
	AVisual *visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	vx->release();
	vy->release();
	mark->release();
	
	ACocoa_CreateWindow(visual, AMkPoint(100, 100));
	// FIXME: we should assign the result or something ...
	visual->release();
}

@end
