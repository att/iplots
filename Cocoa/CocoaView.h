//
//  CocoaView.h
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AVisual.h"


@interface CocoaView : NSOpenGLView {
	AVisual *visual;
}

- (id)initWithFrame:(NSRect)frame visual: (AVisual*) aVisual;
- (void) drawRect: (NSRect) bounds;
- (void) setAWindow: (AWindow*) aWin; // set the AWindow associated with this view

@end
