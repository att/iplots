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
#import "ABarChart.h"

#import "REngine.h"
#import "ARVector.h"

extern "C" { CocoaWindow *ACocoa_CreateWindow(AVisual *visual, APoint position);  }

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
	ALog("applicationDidFinishLaunching:");
	REngine *eng = REngine::mainEngine();
	RObject *o = eng->parseAndEval("{n<-1e4; x<-rnorm(n)}");
	AMarker *mark = new AMarker(o->length());
	ADataVector *vx = new ARDoubleVector(mark, o);
	o->release();
	o = eng->parseAndEval("y<-rnorm(n)");
	ADataVector *vy = new ARDoubleVector(mark, o);
	o->release();
	
	o = eng->parseAndEval("factor(LETTERS[as.integer(y - min(y))+1L])");
	ARFactorVector *fv = new ARFactorVector(mark, o->value());
	o->release();

	ARect aFrame = AMkRect(0, 0, 400, 300);
	AVisual *visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	
	/* CocoaWindow *window = */ ACocoa_CreateWindow(visual, AMkPoint(50, 100));
	
	// FIXME: we should assign the result or something ...
	visual->release();

	ADataVector *pcv[] = { vx, vy, fv };
	
	visual = new AParallelCoordPlot(NULL, aFrame, 0, 3, pcv);
	ACocoa_CreateWindow(visual, AMkPoint(500, 100));
	visual->release();
	
	visual = new ABarChart(NULL, aFrame, 0, fv);
	ACocoa_CreateWindow(visual, AMkPoint(950, 100));
	visual->release();
	
	fv->release();
	vx->release();
	vy->release();
	mark->release();
}

@end
