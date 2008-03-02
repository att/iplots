//
//  CocoaApp.m
//  Acinonyx
//
//  Created by Simon Urbanek
//  Copyright 2008 Simon Urbanek. All rights reserved.
//

#import "CocoaApp.h"
#import "CocoaWindow.h"

@implementation CocoaApp

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	NSRect rect = NSMakeRect(100, 100, 400, 300);
	CocoaWindow *window = [[CocoaWindow alloc] initWithContentRect:rect];
    [window makeKeyAndOrderFront:nil];
	[window setDelegate:self];
}

@end
