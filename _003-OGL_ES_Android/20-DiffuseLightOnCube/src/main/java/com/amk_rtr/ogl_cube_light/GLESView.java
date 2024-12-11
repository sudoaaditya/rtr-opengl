package com.amk_rtr.ogl_cube_light;

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
    private int vao_cube[] = new int[1];
    private int vbo_cubePos[] = new int[1];
    private int vbo_cubeNormal[] = new int[1];

    private int mvUniform;
    private int projMatUniform;
    private int kdUniform, ldUniform, lightPosUniform;
    private int lPressedUniform;
    private float perspectiveProjMat[] = new float[16];

    // Animation Variables!.
    private boolean bLighting = false;
    private boolean bAnimation = false;
    private float fRotAngle = 0.0f;
    private float lightPos[] = new float[]{0.0f, 0.0f, 2.0f, 1.0f};

    
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
        bAnimation = bAnimation == true ? false :  true;
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent event) {
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {
        bLighting = bLighting == true ? false : true;
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
        if(bAnimation == true){
            update();
        }
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
            "precision mediump float;" +
            "precision lowp int;" +
            "in vec4 vPosition;" +
            "in vec3 vNormal;" +
            "uniform mat4 u_mvMatrix;" +
            "uniform mat4 u_projMatrix;" +
            "uniform vec3 u_ld;" +
            "uniform vec3 u_kd;" +
            "uniform int u_lKeyPressed;" +
            "uniform vec4 u_lightPosition;" +
            "out vec3 diffuseColor;" +
            "void main(void) {" +
            "if(u_lKeyPressed == 1) {" +
            "   vec4 eyeCoords = u_mvMatrix * vPosition;" +
            "   mat3 normMatrix = mat3(transpose(inverse(u_mvMatrix)));" +
            "   vec3 tNorm = normalize(normMatrix * vNormal);" +
            "   vec3 s = vec3(u_lightPosition - eyeCoords);" +
            "   diffuseColor = u_ld * u_kd * max(dot(s, tNorm), 0.0);" +
            "}" +
            "gl_Position = u_projMatrix * u_mvMatrix * vPosition;" +
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
            "precision mediump float;" +
            "precision lowp int;" +
            "out vec4 FragColor;" +
            "uniform int u_lKeyPressed;" +
            "in vec3 diffuseColor;" +
            "void main(void) {" +
            "   if(u_lKeyPressed == 1) {" +
            "       FragColor = vec4(diffuseColor, 1.0);" +
            "   }" +
            "   else {" +
            "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
            "   }"+
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
        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

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

        mvUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_mvMatrix");
        projMatUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_projMatrix");
        ldUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ld");
        kdUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_kd");
        lPressedUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lKeyPressed");
        lightPosUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lightPosition");

        //Cube Vertices!
        final float cubeVert[] = new float[] {
            1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f
        };
        //Cube Normals
        final float cubeNorms[] = new float[]{
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f
        };

        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);

        GLES32.glGenBuffers(1, vbo_cubePos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cubePos[0]);
        FloatBuffer posBuff = createFloatBuffer(cubeVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (cubeVert.length * 4), posBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
        GLES32.glGenBuffers(1, vbo_cubeNormal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cubeNormal[0]);
        FloatBuffer normBuff = createFloatBuffer(cubeNorms);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (cubeNorms.length * 4), normBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

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
        float translateMat[] = new float[16];
        float rotateMat[] = new float[16];
        float scaleMat[] = new float[16];
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for CUBE!
        Matrix.setIdentityM(modelViewMat, 0);
        Matrix.setIdentityM(translateMat, 0);
        Matrix.setIdentityM(rotateMat, 0);
        Matrix.setIdentityM(scaleMat, 0);

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -5.0f);
        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0);
        
        Matrix.scaleM(scaleMat, 0, 0.75f, 0.75f, 0.75f);
        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, scaleMat, 0); 

        Matrix.setRotateM(rotateMat, 0, fRotAngle, 1.0f, 1.0f, 1.0f);
        Matrix.multiplyMM(modelViewMat, 0, modelViewMat, 0, rotateMat, 0);

        GLES32.glUniformMatrix4fv(mvUniform, 1, false, modelViewMat, 0);
        GLES32.glUniformMatrix4fv(projMatUniform, 1, false, perspectiveProjMat, 0);

        if(bLighting == true) {
            GLES32.glUniform1i(lPressedUniform, 1);
            GLES32.glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
            GLES32.glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
            GLES32.glUniform4fv(lightPosUniform, 1, lightPos, 0);
        }
        else {
            GLES32.glUniform1i(lPressedUniform, 0);
        }

        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);

        requestRender();
    }

    private void update() {
        fRotAngle += 0.5f;
        if(fRotAngle >= 360.0f) {
            fRotAngle = 0.0f;
        }
    }

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");

        if(vbo_cubePos[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_cubePos, 0);
            vbo_cubePos[0] = 0;
        }

        if(vbo_cubeNormal[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_cubeNormal, 0);
            vbo_cubeNormal[0] = 0;
        }

        if(vao_cube[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_cube, 0);
            vao_cube[0] = 0;
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

