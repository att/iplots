//
//  CocoaWindow.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaWindow.h"
#import "AWindow.h"
#import "GLString.h"

class ACocoaWindow : public AWindow {
	CocoaWindow *window;
public:
	ACocoaWindow(CocoaWindow *window, ARect frame) : AWindow(frame) {
		this->window = window;
	}
	
	virtual void redraw() {
		[window redraw];
	}
	
	virtual void glstring(APoint pt, APoint adj, const char *txt) {
		NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSFont userFontOfSize:20] forKey:NSFontAttributeName];
		NSAttributedString *str = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:txt] attributes:attr];
		//NSAttributedString *str = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:txt]];
		GLString *gs = [[GLString alloc] initWithAttributedString:str];
		NSPoint loc = NSMakePoint(pt.x, pt.y);
		[gs genTexture];
		NSSize ts = [gs texSize];
		// rendering hack - we generate a texture double the size to achieve nicer results
		ts.width *= 0.5; ts.height *= 0.5;
		if (adj.x) loc.x -= ts.width * adj.x;
		if (adj.y) loc.y -= ts.height * adj.y;
		[gs drawAtPoint:loc];
		[gs release];
		[str release];
	}
};

@implementation CocoaWindow

- (id) initWithContentRect: (NSRect) rect visual: (AVisual*) aVisual
{
	self = [super initWithContentRect:rect
							styleMask:NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask
							  backing:NSBackingStoreRetained
								defer:YES];
	if (self) {
		aWindow = new ACocoaWindow(self, AMkRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height));
		aWindow->setRootVisual(aVisual);
		NSRect frame = [self contentRectForFrameRect:[self frame]];
		view = [[CocoaView alloc] initWithFrame:frame visual:aVisual];
		[view setAWindow:aWindow];
		[self setOpaque:YES];
		[self setContentView:view];
		[self makeFirstResponder:view];
		[self setContentMinSize:NSMakeSize(150.0, 100.0)];
	}
	return self;
}

- (void) redraw
{
	NSLog(@"%@: request redraw", self);
	if (view) [view setNeedsDisplay:YES];
}

- (void) dealloc
{
	NSLog(@"%@: dealloc", self);
	if (view) [view release];
	if (aWindow) aWindow->release();
	[super dealloc];
}


@end
