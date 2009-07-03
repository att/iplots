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
	NSFont *font;
	char *font_name;
	double font_size;
public:
	ACocoaWindow(CocoaWindow *window, ARect frame) : AWindow(frame) {
		this->window = window;
		font_name = strdup("Arial");
		font_size = 10.0;
		font = [NSFont fontWithName:[NSString stringWithUTF8String:font_name] size:font_size];
		if (!font) font = [NSFont userFontOfSize:font_size];
		if (font) [font retain];
	}

	virtual ~ACocoaWindow() {
		if (font) [font release];
		if (font_name) free(font_name);
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
		NSDictionary *attr = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
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
	
	virtual void glfont(const char *name, AFloat size) {
		bool changed = false;
		if (size > 0.0 && size != font_size) {
			changed = true;
			font_size = size;
		}
		if (name && strcmp(name, font_name)) {
			free(font_name);
			font_name = strdup(name);
			changed = true;
		}
		if (changed) {
			NSFont *candidate = nil;
			if (*font_name)
				candidate = [NSFont fontWithName:[NSString stringWithUTF8String:font_name] size: font_size];
			if (!candidate) candidate = [NSFont userFontOfSize: font_size];
			if (candidate) {
#ifdef DEBUG
				NSLog(@"new font: %@ replacing %@", candidate, font);
#endif
				[font release];
				font = [candidate retain];
			}
		}
	}
	
	virtual ASize glbbox(const char *txt) {
		NSDictionary *attr = [[NSDictionary alloc] initWithObjectsAndKeys:font, NSFontAttributeName, nil];
		NSString *ns = [[NSString alloc] initWithUTF8String:txt];
		NSAttributedString *str = [[NSAttributedString alloc] initWithString:ns attributes:attr];
		NSSize ss = [str size];
		[str release];
		[attr release];
		[ns release];
		return AMkSize(ss.width, ss.height);;
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
		heartbeatTimer = [[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(heartbeat:) userInfo:nil repeats:YES] retain];
	}
	return self;
}

- (void) heartbeat: (id) sender
{
	//NSLog(@"%@ heartbeat: aWindow=%p, view=%p, isDirty=")
	int *df;
	if (aWindow && view && (df = aWindow->dirtyFlag) && df[0]) {
		df[0]++;
		if (df[0] > 2)
			[view setNeedsDisplay:YES];
	}
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
	[heartbeatTimer invalidate];
	[heartbeatTimer release];
	if (view) [view release];
	if (aWindow) aWindow->release();
	[super dealloc];
}

@end
