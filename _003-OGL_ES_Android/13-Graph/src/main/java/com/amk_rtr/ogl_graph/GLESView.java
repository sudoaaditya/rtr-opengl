package com.amk_rtr.ogl_graph;

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

//for Buffers.
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
//For OGL Matric Maths
import android.opengl.Matrix;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener {

    private GestureDetector gestureDetector;
    private final Context context;
    //OGL Paras
    private int vertexShaderObj;
    private int fragmentShaderObj;
    private int shaderProgObj;
    //vaos & vbos
    private int vaoAxes[] = new int[1];
    private int vboAxesPos[] = new int[1];

    private int vaoHorUp[] = new int[1];
    private int vboHorUpPos[] = new int[1];
    private int vaoHorDwn[] = new int[1];
    private int vboHorDwnPos[] = new int[1];

    private int vaoVertRt[] = new int[1];
    private int vboVertRtPos[] = new int[1];
    private int vaoVertLft[] = new int[1];
    private int vboVertLftPos[] = new int[1];

    private int mvpUniform;
    private float perspectiveProjMat[] = new float[16];
    
    public GLESView(Context drawingContext) {
        super(drawingContext);

        context = drawingContext;

        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(drawingContext, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override 
    public boolean onTouchEvent(MotionEvent event) {


        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);

        return(true);   
    }

    @Override
    public boolean onDoubleTap(MotionEvent event) {
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent event) {
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {
        return(true);
    }

    @Override
    public boolean onDown(MotionEvent event) {
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent eventOne, MotionEvent eventTwo, float velX, float velY) {
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent event) {
        
    }

    @Override
    public boolean onScroll(MotionEvent eventOne, MotionEvent eventTwo, float distX, float distY) {
        uninitialize();
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
        System.out.println("RTR: OpenglES Version : "+version);

        String langVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        System.out.println("RTR: OpenglES Shader Version : "+langVersion);

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

        int iShaderCompStat[] = new int[1];
        int iInfoLogLen[] = new int[1];
        String szInfoLog = null;

        vertexShaderObj =  GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        
        final String vertexShaderSource = String.format
        (
            "#version 320 es" +
            "\n" +
            "in vec4 vPosition;" +
            "in vec4 vColor;" +
            "out vec4 out_color;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void)" +
            "{" +
            "gl_Position = u_mvp_matrix * vPosition;" +
            "out_color = vColor;" +
            "}"    
        );
        
        GLES32.glShaderSource(vertexShaderObj, vertexShaderSource);
        GLES32.glCompileShader(vertexShaderObj);
        
        GLES32.glGetShaderiv(vertexShaderObj, GLES32.GL_COMPILE_STATUS, iShaderCompStat, 0);
        
        if(iShaderCompStat[0] == GLES32.GL_FALSE) {
            GLES32.glGetShaderiv(vertexShaderObj, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLen, 0);
            
            if(iInfoLogLen[0] > 0) {
                szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObj);
                System.out.println("RTR: Vertex Shader Log : " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }
        else {
            System.out.println("RTR: VertexShader Compiled Successfully!!");
        }
        fragmentShaderObj = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        final String fragmentShaderSource = String.format
        (
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "in vec4 out_color;" +
            "out vec4 FragColor;" +
            "void  main(void) {" +
            "FragColor = out_color;" +
            "}"
        );

        GLES32.glShaderSource(fragmentShaderObj, fragmentShaderSource);
        GLES32.glCompileShader(fragmentShaderObj);

        iShaderCompStat[0] = 0;
        iInfoLogLen[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObj, GLES32.GL_COMPILE_STATUS, iShaderCompStat, 0);
        if(iShaderCompStat[0] == GLES32.GL_FALSE) {
            GLES32.glGetShaderiv(fragmentShaderObj, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLen, 0);
            if(iInfoLogLen[0] > 0) {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObj);
                System.out.println("RTR: Fragment Shader Log : " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }
        else {
            System.out.println("RTR: Fragment Shader Compiled Successfully!!");
        }

        shaderProgObj = GLES32.glCreateProgram();
        GLES32.glAttachShader(shaderProgObj, vertexShaderObj);
        GLES32.glAttachShader(shaderProgObj, fragmentShaderObj);

        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_COLOR, "vColor");

        GLES32.glLinkProgram(shaderProgObj);

        int iProgLinkStat[] = new int[1];
        iInfoLogLen[0] = 0;
        szInfoLog = null;
        GLES32.glGetProgramiv(shaderProgObj, GLES32.GL_LINK_STATUS, iProgLinkStat, 0);
        if(iProgLinkStat[0] == GLES32.GL_FALSE) {
            GLES32.glGetProgramiv(shaderProgObj, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLen, 0);
            if(iInfoLogLen[0] > 0) {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgObj);
                System.out.println("RTR: Program Link Log : " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }
        else {
            System.out.println("RTR: Program linked Successfully!!");
        }
        
        mvpUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_mvp_matrix");

        //Vao & Vbo Creation!!.
        final float axesVert[] = new float[]{1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f};
        final float horLinesVert[] = new float [40];
        float fSteps = 0.05f;
        for(int i = 0; i< 10; i++) {
            for(int j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVert[(i*4) + j] = 1.0f;
                }
                else if(j == 2) {
                    horLinesVert[(i * 4) + j] = -1.0f;
                }
                else {
                    horLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.05f;
        }
        final float horLinesVertDwn[] = new float [40];
        fSteps = 0.05f;
        for(int i = 0; i< 10; i++) {
            for(int j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVertDwn[(i*4) + j] = 1.0f;
                }
                else if(j == 2) {
                    horLinesVertDwn[(i * 4) + j] = -1.0f;
                }
                else {
                    horLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.05f;
        }
        final float vertLinesVert[] = new float [80];
        fSteps = 0.05f;
        for(int i = 0; i< 20; i++) {
            for(int j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVert[(i*4) + j] = 1.0f;
                }
                else if(j == 3) {
                    vertLinesVert[(i * 4) + j] = -1.0f;
                }
                else {
                    vertLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.05f;
        }
        final float vertLinesVertDwn[] = new float [80];
        fSteps = 0.05f;
        for(int i = 0; i< 20; i++) {
            for(int j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVertDwn[(i*4) + j] = 1.0f;
                }
                else if(j == 3) {
                    vertLinesVertDwn[(i * 4) + j] = -1.0f;
                }
                else {
                    vertLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.05f;
        }

        //Axes
        GLES32.glGenVertexArrays(1, vaoAxes, 0);
        GLES32.glBindVertexArray(vaoAxes[0]);
        GLES32.glGenBuffers(1, vboAxesPos,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboAxesPos[0]);
        FloatBuffer axesPosBuff = createFloatBuffer(axesVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (axesVert.length * 4), axesPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); 
        GLES32.glVertexAttrib3f(GLESMacros.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
        GLES32.glBindVertexArray(0);
        //Hor Lines Up
        GLES32.glGenVertexArrays(1, vaoHorUp, 0);
        GLES32.glBindVertexArray(vaoHorUp[0]);
        GLES32.glGenBuffers(1, vboHorUpPos,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboHorUpPos[0]);
        FloatBuffer horUpPosBuff = createFloatBuffer(horLinesVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (horLinesVert.length * 4), horUpPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); 
        GLES32.glVertexAttrib3f(GLESMacros.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
        GLES32.glBindVertexArray(0);
        //Hor Lines Down
        GLES32.glGenVertexArrays(1, vaoHorDwn, 0);
        GLES32.glBindVertexArray(vaoHorDwn[0]);
        GLES32.glGenBuffers(1, vboHorDwnPos,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboHorDwnPos[0]);
        FloatBuffer horDwnPosBuff = createFloatBuffer(horLinesVertDwn);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (horLinesVertDwn.length * 4), horDwnPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); 
        GLES32.glVertexAttrib3f(GLESMacros.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
        GLES32.glBindVertexArray(0);
        //Vert Lines Rt
        GLES32.glGenVertexArrays(1, vaoVertRt, 0);
        GLES32.glBindVertexArray(vaoVertRt[0]);
        GLES32.glGenBuffers(1, vboVertRtPos,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboVertRtPos[0]);
        FloatBuffer vertRtPosBuff = createFloatBuffer(vertLinesVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (vertLinesVert.length * 4), vertRtPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); 
        GLES32.glVertexAttrib3f(GLESMacros.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
        GLES32.glBindVertexArray(0);
        //Vert Lines Lft
        GLES32.glGenVertexArrays(1, vaoVertLft, 0);
        GLES32.glBindVertexArray(vaoVertLft[0]);
        GLES32.glGenBuffers(1, vboVertLftPos,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboVertLftPos[0]);
        FloatBuffer vertLftPosBuff = createFloatBuffer(vertLinesVertDwn);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (vertLinesVertDwn.length * 4), vertLftPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0); 
        GLES32.glVertexAttrib3f(GLESMacros.AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);
        GLES32.glBindVertexArray(0);
        
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        Matrix.setIdentityM(perspectiveProjMat, 0);
    
    }

    private FloatBuffer createFloatBuffer(float fArr[]) {
        int memSize = (fArr.length * 4);
        //Step 1 : Allocate Native Memory
        ByteBuffer byteBuff = ByteBuffer.allocateDirect(memSize);
        //Step 2 : Now Cange Order of Buffer to native One
        byteBuff.order(ByteOrder.nativeOrder());
        //Step 3 : Create Float Buffer
        FloatBuffer floatBuffer = byteBuff.asFloatBuffer();
        //Step 4 : Now put Array into the "Coocked Buffer"
        floatBuffer.put(fArr);
        //Step 5 : Set the Array at index 0 in buffer
        floatBuffer.position(0);
        return(floatBuffer);
    }

    private void resize(int width, int height) {

        if(height == 0) {
            height = 1;
        }
        GLES32.glViewport(0, 0, width, height);
        Matrix.perspectiveM(perspectiveProjMat, 0, 45.0f, (width/height), 0.1f, 100.0f);
    }

    private void display() {
        float modelViewMat[] = new float[16];
        float modelViewProjMat[] = new float[16];
        float translateMat[] = new float[16];
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for triangle!
        Matrix.setIdentityM(modelViewProjMat, 0);
        Matrix.setIdentityM(modelViewMat, 0);
        Matrix.setIdentityM(translateMat, 0);

        Matrix.translateM(translateMat, 0, 0.f, 0.0f, -1.0f);

        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0); 
        Matrix.multiplyMM(modelViewProjMat, 0 , perspectiveProjMat, 0, modelViewMat, 0);
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjMat, 0);

        GLES32.glLineWidth(4.0f);
        GLES32.glBindVertexArray(vaoAxes[0]);
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 2, 2);
        GLES32.glBindVertexArray(0);

        GLES32.glLineWidth(0.8f);
        GLES32.glBindVertexArray(vaoHorUp[0]);
        for(int i = 0; i< 20; i += 2) {
            GLES32.glDrawArrays(GLES32.GL_LINES, i, 2);
        }
        GLES32.glBindVertexArray(0);

        GLES32.glBindVertexArray(vaoHorDwn[0]);
        for(int i = 0; i< 20; i += 2) {
            GLES32.glDrawArrays(GLES32.GL_LINES, i, 2);
        }
        GLES32.glBindVertexArray(0);

        GLES32.glBindVertexArray(vaoVertRt[0]);
        for(int i = 0; i< 40; i += 2) {
            GLES32.glDrawArrays(GLES32.GL_LINES, i, 2);
        }
        GLES32.glBindVertexArray(0);

        GLES32.glBindVertexArray(vaoVertLft[0]);
        for(int i = 0; i< 40; i += 2) {
            GLES32.glDrawArrays(GLES32.GL_LINES, i, 2);
        }
        GLES32.glBindVertexArray(0);    

        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");
        if(vboAxesPos[0] != 0) {
            GLES32.glDeleteBuffers(1, vboAxesPos, 0);
            vboAxesPos[0] = 0;
        }
        if(vaoAxes[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vaoAxes, 0);
            vaoAxes[0] = 0;
        }
        
        if(shaderProgObj != 0) {
            int iShaderCnt[] = new int[1];
            int itr;
            GLES32.glUseProgram(shaderProgObj);

            GLES32.glGetProgramiv(shaderProgObj, GLES32.GL_ATTACHED_SHADERS, iShaderCnt, 0);

            int shaders[] = new int[iShaderCnt[0]];

            GLES32.glGetAttachedShaders(shaderProgObj, iShaderCnt[0], iShaderCnt, 0, shaders, 0);

            for(itr = 0; itr < iShaderCnt[0]; itr++) {
                GLES32.glDetachShader(shaderProgObj, shaders[itr]);
                GLES32.glDeleteShader(shaders[itr]);
                shaders[itr] = 0;
            }
            GLES32.glUseProgram(0);
            GLES32.glDeleteProgram(shaderProgObj);
            shaderProgObj = 0;
        }   
    }
}

