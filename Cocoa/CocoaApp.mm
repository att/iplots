//
//  CocoaApp.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaApp.h"
#import "CocoaWindow.h"

#import "AScatterPlot.h"
#import "AParallelCoordPlot.h"

#import "REngine.h"

CocoaWindow *ACocoa_CreateWindow(AVisual *visual, APoint position)
{
	ARect aFrame = visual->frame();
	NSRect rect = NSMakeRect(position.x, position.y, aFrame.width, aFrame.height);

	CocoaWindow *window = [[CocoaWindow alloc] initWithContentRect:rect visual:visual];

	[window setTitle:[NSString stringWithUTF8String:visual->describe()]];
    [window makeKeyAndOrderFront:nil];
	// [window setDelegate:self];
	// no idea why, but the shadow is not there until you cycle the shadow setting...
	//[window setHasShadow:NO];
	//[window setHasShadow:YES];
	//[window setReleasedWhenClosed:YES];

	return window;
}

@implementation CocoaApp

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	REngine *eng = REngine::mainEngine();
	RObject *o = eng->parseAndEval("rnorm(1e4)");
	AMarker *mark = new AMarker(o->length());
	ADataVector *vx = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
	o = eng->parseAndEval("rnorm(1e4)");	
	ADataVector *vy = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
	
	ARect aFrame = AMkRect(0, 0, 400, 300);
	AVisual *visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	
	/* CocoaWindow *window = */ ACocoa_CreateWindow(visual, AMkPoint(100, 100));
	
	// FIXME: we should assign the result or something ...
	visual->release();

	ADataVector *pcv[] = { vx, vy };
	
	visual = new AParallelCoordPlot(NULL, aFrame, 0, 2, pcv);
	ACocoa_CreateWindow(visual, AMkPoint(550, 100));
	visual->release();
	
	vx->release();
	vy->release();
	mark->release();
	
}

@end
