//
//  MyView.m
//  Window
//
//  Created by Ananth Chandrasekharan on 09/01/20.
//

#import "MyView.h"

@implementation MyView {
    
    NSString *centralText;
}

-(id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    
    if(self) {
        //Set BKGND COlor
        [self setBackgroundColor:[UIColor whiteColor]];
        
        centralText = @"Hello World!!..";
        
        //Gesture Recognition!
        //Single Tap
        UITapGestureRecognizer *singleTapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(onSingleTap:)];
        [singleTapRecog setNumberOfTapsRequired:1];
        [singleTapRecog setNumberOfTouchesRequired:1];
        [singleTapRecog setDelegate:self];
        [self addGestureRecognizer:singleTapRecog];
        
        //Double Tap
        UITapGestureRecognizer *doubleTapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(onDoubleTap:)];
        [doubleTapRecog setNumberOfTapsRequired:2];
        [doubleTapRecog setNumberOfTouchesRequired:1];
        [doubleTapRecog setDelegate:self];
        [self addGestureRecognizer:singleTapRecog];
        
        //On Double Tap, Single
        [singleTapRecog requireGestureRecognizerToFail:doubleTapRecog];
        
        //Swipe Gesture!
        UISwipeGestureRecognizer *swipeRecog = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
        [self addGestureRecognizer:swipeRecog];
        
        //Long Press!
        UILongPressGestureRecognizer *longPressRecog = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:longPressRecog];
        
    }
    
    return self;
}

//Draw Rect
-(void) drawRect:(CGRect)dirtyRect {
    
    UIColor *bkGnd = [UIColor blackColor];
    [bkGnd set];
    UIRectFill(dirtyRect);
    
    //Dictonary With KVC
    //Srtting Font Data in Dictionary!.
    NSDictionary *dictTextAttrib = [NSDictionary dictionaryWithObjectsAndKeys: [UIFont fontWithName:@"Helvetica" size:24], NSFontAttributeName,
                                    [UIColor greenColor], NSForegroundColorAttributeName, nil];
    
    CGSize textSize = [centralText sizeWithAttributes: dictTextAttrib];
    
    CGPoint point;
    point.x = (dirtyRect.size.width/2)-(textSize.width/2);
    point.y = (dirtyRect.size.height/2)-(textSize.height/2) + 12;
    
    [centralText drawAtPoint: point withAttributes:dictTextAttrib];

}

-(BOOL) acceptsFirstResponder{
    return(YES);
}

-(void) touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    //TO Start Our Code for accepting the
}

-(void) onSingleTap:(UITapGestureRecognizer *) event {
    centralText = @"Single Tap!.";
    [self setNeedsDisplay];
}

-(void) onDoubleTap:(UITapGestureRecognizer *) event {
    centralText = @"Double Tap!.";
    [self setNeedsDisplay];
}

-(void) onSwipe:(UISwipeGestureRecognizer *) event {
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *) event {
    centralText = @"Long Press!.";
    [self setNeedsDisplay];
}

-(void) dealloc {
    [super dealloc];
}

@end

