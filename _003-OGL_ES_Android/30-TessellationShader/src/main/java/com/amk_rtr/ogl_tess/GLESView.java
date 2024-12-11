package com.amk_rtr.ogl_tess;

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
    private int tessControlShaderObj;
    private int tessEvalShaderObj;
    private int fragmentShaderObj;
    private int shaderProgObj;

    private int vao[] = new int[1];
    private int vbo[] = new int[1];
    
    private int mvpUniform;
    private int numSegmentUniform;
    private int numStripsUniform;
    private int lineColorUniform;
    private float perspectiveProjMat[] = new float[16];

    private int numLineSegments = 1;
    
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
        numLineSegments--;
        if(numLineSegments <= 0) {
            numLineSegments = 1;
        }
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {
        numLineSegments++;
        if(numLineSegments >= 50) {
            numLineSegments = 50;
        }
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
            "in vec2 vPosition;" +
            "void main(void)" +
            "{" +
            "   gl_Position = vec4(vPosition,0.0,1.0);" +
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

        tessControlShaderObj =  GLES32.glCreateShader(GLES32.GL_TESS_CONTROL_SHADER);
        
        final String tessControlShaderSource = String.format
        (
            "#version 320 es" +
            "\n" +
            "layout(vertices=4)out;" +
            "uniform int numberOfSegments;" +
            "uniform int numberOfStrips;" +
            "void main(void)" +
            "{" +
            "   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;" +
            "   gl_TessLevelOuter[0] = float(numberOfStrips);" +
            "   gl_TessLevelOuter[1] = float(numberOfSegments);" +
            "}"    
        );
        
        GLES32.glShaderSource(tessControlShaderObj, tessControlShaderSource);
        GLES32.glCompileShader(tessControlShaderObj);
        
        iShaderCompStat[0] = 0;
        iInfoLogLen[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessControlShaderObj, GLES32.GL_COMPILE_STATUS, iShaderCompStat, 0);
        
        if(iShaderCompStat[0] == GLES32.GL_FALSE) {
            GLES32.glGetShaderiv(tessControlShaderObj, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLen, 0);
            
            if(iInfoLogLen[0] > 0) {
                szInfoLog = GLES32.glGetShaderInfoLog(tessControlShaderObj);
                System.out.println("RTR: Tessellation Control Shader Log : " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }
        else {
            System.out.println("RTR: Tessellation Control Shader Compiled Successfully!!");
        }

        tessEvalShaderObj =  GLES32.glCreateShader(GLES32.GL_TESS_EVALUATION_SHADER);

        final String tessEvalShaderSource = String.format
        (
            "#version 320 es" +
            "\n" +
            "layout(isolines)in;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void)" +
            "{" +
            "   float u = gl_TessCoord.x;" +
            "   vec3 p0 = gl_in[0].gl_Position.xyz;" +
            "   vec3 p1 = gl_in[1].gl_Position.xyz;" +
            "   vec3 p2 = gl_in[2].gl_Position.xyz;" +
            "   vec3 p3 = gl_in[3].gl_Position.xyz;" +
            "   float u1 = (1.0-u);" +
            "   float u2 = u * u;" +
            "   float b3 =  u2 * u;" +
            "   float b2 = 3.0 * u2 * u1;" +
            "   float b1 = 3.0 * u * u1 * u1;" +
            "   float b0 = u1 * u1 * u1;" +
            "   vec3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;" +
            "   gl_Position = u_mvp_matrix * vec4(p,1.0);" +
            "}"    
        );
        
        GLES32.glShaderSource(tessEvalShaderObj, tessEvalShaderSource);
        GLES32.glCompileShader(tessEvalShaderObj);
        
        iShaderCompStat[0] = 0;
        iInfoLogLen[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessEvalShaderObj, GLES32.GL_COMPILE_STATUS, iShaderCompStat, 0);
        
        if(iShaderCompStat[0] == GLES32.GL_FALSE) {
            GLES32.glGetShaderiv(tessEvalShaderObj, GLES32.GL_INFO_LOG_LENGTH, iInfoLogLen, 0);
            
            if(iInfoLogLen[0] > 0) {
                szInfoLog = GLES32.glGetShaderInfoLog(tessEvalShaderObj);
                System.out.println("RTR: Tessellation Evaluation Shader Log : " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }
        else {
            System.out.println("RTR: Tessellation Evaluation Shader Compiled Successfully!!");
        }

        fragmentShaderObj = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        
        final String fragmentShaderSource = String.format
        (
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "uniform vec4 lineColor;" +
            "out vec4 FragColor;" +
            "void main(void)" +
            "{" +
            "FragColor = lineColor;" +
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
        GLES32.glAttachShader(shaderProgObj, tessControlShaderObj);
        GLES32.glAttachShader(shaderProgObj, tessEvalShaderObj);
        GLES32.glAttachShader(shaderProgObj, fragmentShaderObj);

        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");

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
        mvpUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_mvp_matrix");
        numSegmentUniform = GLES32.glGetUniformLocation(shaderProgObj, "numberOfSegments");    
        numStripsUniform = GLES32.glGetUniformLocation(shaderProgObj, "numberOfStrips");
        lineColorUniform = GLES32.glGetUniformLocation(shaderProgObj, "lineColor");

        //Now Usual
        final float lineVertices[] = new float[]{ -1.0f, -1.0f, -0.5f, 1.0f, 0.5f, -1.0f, 1.0f, 1.0f };

        GLES32.glGenVertexArrays(1, vao, 0);
        GLES32.glBindVertexArray(vao[0]);
        GLES32.glGenBuffers(1, vbo, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);
        //Create Buffer that is eligible to sent oer GLBufferData
        FloatBuffer posBuffer = creteFloatBuffer(lineVertices);
        //glBuffdata
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (lineVertices.length * 4), posBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        Matrix.setIdentityM(perspectiveProjMat, 0);
    
    }

    private FloatBuffer creteFloatBuffer(float fArr[]) {
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
        float lineCol[] = new float[]{0.5f, 0.2f, 0.7f, 1.0f};
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);
        
        Matrix.setIdentityM(modelViewProjMat, 0);
        Matrix.setIdentityM(modelViewMat, 0);
        Matrix.setIdentityM(translateMat, 0);

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -4.0f);

        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0); 
        Matrix.multiplyMM(modelViewProjMat, 0 , perspectiveProjMat, 0, modelViewMat, 0);
        
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjMat, 0);
        GLES32.glUniform1i(numSegmentUniform, numLineSegments);
        GLES32.glUniform1i(numStripsUniform, 1);
        GLES32.glUniform4f(lineColorUniform, 0.5f, 0.2f, 0.7f, 1.0f);

        GLES32.glPatchParameteri(GLES32.GL_PATCH_VERTICES, 4);

        GLES32.glBindVertexArray(vao[0]);
        GLES32.glDrawArrays(GLES32.GL_PATCHES, 0, 4);
        GLES32.glBindVertexArray(0);
        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");
        if(vbo[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo, 0);
            vbo[0] = 0;
        }

        if(vao[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
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

