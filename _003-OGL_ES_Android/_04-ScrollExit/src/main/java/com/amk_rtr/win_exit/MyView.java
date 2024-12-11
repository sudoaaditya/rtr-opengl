package com.amk_rtr.win_exit;

// Added By ME!
import androidx.appcompat.widget.AppCompatTextView; //for AppCompatTextView Class.
import android.content.Context; //for Context Class.
import android.view.Gravity;    //for Gravity Class.
import android.graphics.Color;  //for Color Class.
//for Event Hand
import android.view.MotionEvent;    //for MotionEvent class.
import android.view.GestureDetector;    //for GestureDetector Class
import android.view.GestureDetector.OnGestureListener;  //for OnGestureListener class.
import android.view.GestureDetector.OnDoubleTapListener;    //for OnDoubleTapListener class

public class MyView extends AppCompatTextView implements OnGestureListener, OnDoubleTapListener {

    private GestureDetector gestureDetector;

    public MyView(Context drawingContext) {
        
        super(drawingContext);

        setTextColor(Color.rgb(255, 128, 0));
        setTextSize(40);
        setGravity(Gravity.CENTER);
        setText(" Event Handler ");

        //Created Instance on Gesturedetector and allowd him to monitor double tap too!
        gestureDetector = new GestureDetector(drawingContext, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override 
    public boolean onTouchEvent(MotionEvent event) {

        //Not to use in whole OGL but Keyboard ecent are Accepted like so!
        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);

        return(true);   //accepting and delegating calls to OnDoubleTapListener
    }

    // method from OnDoubleTapListener
    @Override
    public boolean onDoubleTap(MotionEvent event) {

        setText("Double Tap!.");
        return(true);
    }

    // method from OnDoubleTapListener
    @Override
    public boolean onDoubleTapEvent(MotionEvent event) {
        //Similar to onDoubleTap so need not to write anthing here!
        return(true);
    }

    // method from OnDoubleTapListener
    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {

        setText("Single Tap!.");
        return(true);
    }

    // method from OnGestureListener
    @Override
    public boolean onDown(MotionEvent event) {
        //Similar to onSingleTapConfirmed so need not to write anthing here!
        return(true);
    }

    // method from OnGestureListener
    @Override
    public boolean onFling(MotionEvent eventOne, MotionEvent eventTwo, float velX, float velY) {
        //Swipe Like
        return(true);
    }

    // method from OnGestureListener
    @Override
    public void onLongPress(MotionEvent event) {
        
        setText("Long Press!.");
        
    }

    // method from OnGestureListener
    @Override
    public boolean onScroll(MotionEvent eventOne, MotionEvent eventTwo, float distX, float distY) {
        System.exit(0);
        return(true);
    }

    // method from OnGestureListener
    @Override
    public void onShowPress(MotionEvent event) {

    }

    // method from OnGestureListener
    @Override
    public boolean onSingleTapUp(MotionEvent e) {
        return(true);
    }
}



