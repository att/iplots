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
	
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {
#if 0 // double-the-size-code but that doesn't support rotation
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
#else
		NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSFont userFontOfSize:10] forKey:NSFontAttributeName];
		NSAttributedString *str = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:txt] attributes:attr];
		//NSAttributedString *str = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:txt]];
		GLString *gs = [[GLString alloc] initWithAttributedString:str];
		NSPoint loc = NSMakePoint(pt.x, pt.y), adjp = NSMakePoint(adj.x, adj.y);
		[gs genTexture];
		NSSize ts = [gs texSize];
		[gs drawAtPoint:loc withAdjustment:adjp rotation:rot];
		[gs release];
		[str release];
#endif
	}
};

@implementation CocoaWindow

- (id) initWithContentRect: (NSRect) rect visual: (AVisual*) aVisual
{
	self = [super initWithContentRect:rect
							styleMask:NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask
							  backing:NSBackingStoreBuffered//NSBackingStoreRetained
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
		[self setAcceptsMouseMovedEvents:YES];
		[self setContentMinSize:NSMakeSize(150.0, 100.0)];
	}
	return self;
}

- (void) redraw
{
#ifdef DEBUG
	NSLog(@"%@: request redraw", self);
#endif
	if (view) [view setNeedsDisplay:YES];
}

- (void) dealloc
{
#ifdef DEBUG
	NSLog(@"%@: dealloc", self);
#endif
	if (view) [view release];
	if (aWindow) aWindow->release();
	[super dealloc];
}

@end
