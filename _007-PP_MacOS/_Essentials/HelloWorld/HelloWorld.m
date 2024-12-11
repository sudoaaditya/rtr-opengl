//
//  HelloWorld.m
//  
//
//  Created by Aaditya Kashid on 06/01/20.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

//Interface Declaration!.
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface MyView: NSView

@end

//Main
int main(int argc, const char* argv[]){
    
    NSAutoreleasePool *pNsPool = [[NSAutoreleasePool alloc] init];
    
    NSApp = [NSApplication sharedApplication];
    
    [NSApp setDelegate: [[AppDelegate alloc] init]];
    
    [NSApp run];
    
    [pNsPool release];
    
    return(0);
}

//Imterface Implementation.
@implementation AppDelegate {
    @private
    NSWindow *window;
    MyView *view;
}

-(void) applicationDidFinishLaunching:(NSNotification *)aNotify {
    
    //Create Window!.
    NSRect winRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    
    window = [[NSWindow alloc]initWithContentRect:winRect styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |NSWindowStyleMaskResizable backing: NSBackingStoreBuffered defer:NO];
    
    [window setTitle:@"macOS Window!!"];
    [window center];
    
    //Create View Object & Set it as view of out Window
    view = [[MyView alloc]initWithFrame:winRect];
    [window setContentView:view];
    //Tell WIndow that AppDelegate Will Take Charge Onwards && Set Winodw on Focus & Front in ZOrder
    [window setDelegate:self];
    [window makeKeyAndOrderFront: self];
    
}

-(void) applicationWillTerminate:(NSNotification *)aNotify{
    //Code.
    //Here we will take care of LogFile Closing
}

-(void) windowWillClose:(NSNotification *)aNotify {
    //Tell NSApp to close the APP. by closing AppDelegate
    [NSApp terminate:self];
}

-(void) dealloc{
    [view release];
    
    [window release];
    
    [super dealloc];
    
}
@end

@implementation MyView {
    //Not Specifying AccessSpecifer is what declaring Private is. CPP!! MacOS doesn't allow inline initialization.
    NSString *centralText;
}

-(id) initWithFrame:(NSRect) rect {
    
    self =[super initWithFrame:rect];
    
    if(self){
        
        [[self window] setContentView:self];
        
        centralText = @"Hello World!!..";
    }
    return(self);
    
}

-(void) drawRect:(NSRect) dirtyRect {
    //Code!.
    
    //Back BG
    NSColor *fillCol = [NSColor blackColor];
    [fillCol set];
    NSRectFill(dirtyRect);
    
    //Srtting Font Data in Dictionary!.
    NSDictionary *dictTextAttrib = [NSDictionary dictionaryWithObjectsAndKeys: [NSFont fontWithName:@"Helvetica" size:32], NSFontAttributeName,
                                    [NSColor greenColor], NSForegroundColorAttributeName, nil];
    
    NSSize textSize = [centralText sizeWithAttributes: dictTextAttrib];
    
    NSPoint point;
    point.x = (dirtyRect.size.width/2)-(textSize.width/2);
    point.y = (dirtyRect.size.height/2)-(textSize.height/2) + 12;
    
    [centralText drawAtPoint: point withAttributes:dictTextAttrib];

}

//This Step to add our app at the recent most level in Responser chain!.
-(BOOL) acceptsFirstResponder {
    //Code!.
    [[self window] makeFirstResponder: self];
    return(YES);
}

-(void) keyDown:(NSEvent *)event {
    
    //This to Take first key pressed in the multiple keyDowns
    int keyCode = (int)[[event characters] characterAtIndex:0];

    switch (keyCode) {
        case 27: //Esc
            [self release];
            [NSApp terminate:self];
            break;
            
        case 'F':
        case 'f':
            centralText = @"F Pressed!!..";
            [[self window] toggleFullScreen:self]; //Repainting is took care of!
            break;
            
        default:
            break;
    }
    
}

-(void) mouseDown:(NSEvent *) event {
    //Code
    centralText = @"LButtonDown!!..";
    [self setNeedsDisplay:YES]; //InValidateRect..
}

-(void) mouseDragged:(NSEvent *) event {
    //Code
}

-(void) rightMouseDown: (NSEvent *) event {
    centralText = @"RButtonDown!!..";
    [self setNeedsDisplay:YES];
}


-(void) dealloc {
    
    [super dealloc];

}

@end











