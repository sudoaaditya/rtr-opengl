package com.amk_rtr.ogl_bluewindow;

// Added By ME!
import android.content.Context; //for Context Class.
//for Event Hand
import android.view.MotionEvent;    //for MotionEvent class.
import android.view.GestureDetector;    //for GestureDetector Class
import android.view.GestureDetector.OnGestureListener;  //for OnGestureListener class.
import android.view.GestureDetector.OnDoubleTapListener;    //for OnDoubleTapListener class
//for OpenGLES
import android.opengl.GLSurfaceView;    
import android.opengl.GLES32;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener {

    private GestureDetector gestureDetector;
    private final Context context;

    public GLESView(Context drawingContext) {
        
        super(drawingContext);

        context = drawingContext;

        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

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

    //Implement GLSurfaceView Methods
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        String version = gl.glGetString(GL10.GL_VERSION);
        System.out.println("RTR : OpenglES Version : "+version);
        initialize();
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height) {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 unused){
        display();
    }

    //Now Our OGL Methods.
    private void initialize() {
        GLES32.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    }

    private void resize(int width, int height) {
        if(height == 0) {
            height = 1;
        }

        GLES32.glViewport(0, 0, width, height);
    }

    private void display() {

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        requestRender();
    }
}



