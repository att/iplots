//
//  CocoaView.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaView.h"

#include "AScatterPlot.h"

// test class for a visual
class MyVisual : public AVisual {
public:
	MyVisual(ARect frame) : AVisual(NULL, frame, 0) {}
	
	virtual void draw() {
		color(1,1,0.7,1);
		triP(AMkPoint(-1.0, -1.0),AMkPoint(1.0, -1.0),AMkPoint(1.0, 1.0));

		color(1.0f, 0.0f, 0.0f ,0.5f);
		triP(AMkPoint(0,1),AMkPoint(-0.2,-0.3),AMkPoint(0.2, -0.3));
		triP(AMkPoint(-1,0),AMkPoint(1,0),AMkPoint(0,-1));
	}
};

// conversion between Cocoa events and AEvents

static int NSEvent2AEFlags(NSEvent *e) {
	int flags = 0;
	NSUInteger ef = [e modifierFlags];
	if (ef & NSShiftKeyMask) flags |= AEF_SHIFT;
	if (ef & NSControlKeyMask) flags |= AEF_CTRL;
	if (ef & NSAlternateKeyMask) flags |= AEF_ALT;
	if (ef & NSCommandKeyMask) flags |= AEF_META;
	return flags;
}

static APoint NSEventLoc2AEPoint(NSEvent *e) {
	NSPoint pt = [e locationInWindow];
	return AMkPoint(pt.x, pt.y);
}

@implementation CocoaView

- (id)initWithFrame:(NSRect)frame {
	const NSOpenGLPixelFormatAttribute attrs[] = {
//		NSOpenGLPFAAccelerated,
//		NSOpenGLPFAColorSize, 24,
//		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFANoRecovery, NSOpenGLPFASampleBuffers, 1, NSOpenGLPFASamples, 4, /* <- anti-aliasing */
	0 };
    self = [super initWithFrame:frame pixelFormat:[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs]];
    if (self) {
		ARect aFrame = AMkRect(0,0,frame.size.width,frame.size.height);
		// visual = new MyVisual(AMkRect(frame.origin.x,frame.origin.y,frame.size.width,frame.size.height));
		float data_x[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
		float data_y[] = { 1.0, 2.0, 1.5, 3.0, 5.0, 6.0 };		
		AVector *vx = new AFloatVector(data_x, sizeof(data_x)/sizeof(data_x[0]));
		AVector *vy = new AFloatVector(data_y, sizeof(data_y)/sizeof(data_y[0]));
		visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
		vx->release();
		vy->release();
    }
    return self;
}

- (void)drawRect:(NSRect)rect {
	NSRect frame = [self frame];
	// NSLog(@" frame = %f,%f - %f x %f\n", frame.origin.x,frame.origin.y,frame.size.width,frame.size.height);

	/*NSLog(@"OpenGL:\n - vendor = '%s'\n - renderer = '%s'\n - version = '%s'\n - exts = '%s'",
	glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_EXTENSIONS)); */
	
	visual->moveAndResize(AMkRect(frame.origin.x,frame.origin.y,frame.size.width,frame.size.height));
	visual->begin();
	visual->draw();
	visual->end();

#if 0
	glLineWidth(1.0);
	int y = 0;
	while (y < 1000) {
		float x = -1.0f;
		glBegin( GL_LINE_STRIP );
		while (x < 1.0f) {
			glVertex3f(x, sin(x/3.f+((float)y)*0.01), 0.0f);
			x+= 0.01f;
		}
		y++;
		glEnd();
	}
#endif
}

- (void) setAWindow: (AWindow*) aWin
{
	if (visual) visual->window = aWin; // FIXME: memory management?
}

- (void)mouseDown:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_DOWN, NSEvent2AEFlags(theEvent) | AEF_BUTTON1, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_DOWN, NSEvent2AEFlags(theEvent) | AEF_BUTTON2, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_MOVE, NSEvent2AEFlags(theEvent), 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_MOVE, NSEvent2AEFlags(theEvent) | AEF_BUTTON1, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_MOVE, NSEvent2AEFlags(theEvent) | AEF_BUTTON2, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)mouseUp:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_UP, NSEvent2AEFlags(theEvent) | AEF_BUTTON1, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_MOUSE_UP, NSEvent2AEFlags(theEvent) | AEF_BUTTON2, 0, NSEventLoc2AEPoint(theEvent)));
}

- (void)keyDown:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_KEY_DOWN, NSEvent2AEFlags(theEvent), [theEvent keyCode], NSEventLoc2AEPoint(theEvent)));
}

- (void)keyUp:(NSEvent *)theEvent
{
	visual->event(AMkEvent(AE_KEY_UP, NSEvent2AEFlags(theEvent), [theEvent keyCode], NSEventLoc2AEPoint(theEvent)));
}

- (void) dealloc
{
	if (visual) visual->release();
	[super dealloc];
}

@end
