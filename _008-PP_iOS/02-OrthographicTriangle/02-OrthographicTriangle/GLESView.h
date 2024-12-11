//
//  GLESView.h
//  02-OrthographicTriangle
//
//  Created by Ananth Chandrasekharan on 11/01/20.
//

#import <UIKit/UIKit.h>

@interface GLESView : UIView <UIGestureRecognizerDelegate>

-(void) startAnimation;
-(void) stopAnimation;

@end
