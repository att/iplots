//
//  CocoaWindow.h
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CocoaView.h"

//struct ACocoaWindow;

class AVisual;

@interface CocoaWindow : NSWindow
{
	struct ACocoaWindow *aWindow;
	CocoaView *view;
}

- (id) initWithContentRect: (NSRect) rect visual: (AVisual*) aVisual;
- (void) redraw;

@end
