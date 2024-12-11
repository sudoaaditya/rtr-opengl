package com.amk_rtr.ogl_india;

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

    private int vaoI1[] = new int[1];
    private int vboI1Pos[] = new int[1];
    private int vboI1Col[] = new int[1];

    private int vaoN[] = new int[1];
    private int vboNPos[] = new int[1];
    private int vboNCol[] = new int[1];

    private int vaoD[] = new int[1];
    private int vboDPos[] = new int[1];
    private int vboDCol[] = new int[1];

    private int vaoDArc[] = new int[1];
    private int vboDArcPos[] = new int[1];
    private int vboDArcCol[] = new int[1];

    private int vaoI2[] = new int[1];
    private int vboI2Pos[] = new int[1];
    private int vboI2Col[] = new int[1];

    private int vaoA[] = new int[1];
    private int vboAPos[] = new int[1];
    private int vboACol[] = new int[1];

    private int mvpUniform;
    private float perspectiveProjMat[] = new float[16];
    //Color Vars
    //Color MAcros
    //Saffron 255 153 51
    private float SAFFRON_R = 1.0f;
    private float SAFFRON_G = 0.600f;
    private float SAFFRON_B = 0.20f;
    //White 255 255 255
    private float WHITE_R = 1.0f;
    private float WHITE_G = 1.0f;
    private float WHITE_B = 1.0f;
    //Green 18 136 7
    private float GREEN_R = 0.070f;
    private float GREEN_G = 0.533f;
    private float GREEN_B = 0.027f;
    //Blue 0 0 136
    private float BLUE_R = 0.0f;
    private float BLUE_G = 0.0f;
    private float BLUE_B = 0.533f;

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
            "gl_PointSize = 6.4;" +
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
        
        //I
        float I1Vert[] = new float[]{-1.5f, 0.5f, 0.0f, -1.5f, -0.5f, 0.0f};
        float I1Col[] = new float[]{SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B};
        GLES32.glGenVertexArrays(1, vaoI1, 0);
        GLES32.glBindVertexArray(vaoI1[0]);
        GLES32.glGenBuffers(1, vboI1Pos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboI1Pos[0]);
        FloatBuffer i1PosBuff = createFloatBuffer(I1Vert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (I1Vert.length * 4), i1PosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT,false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboI1Col, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboI1Col[0]);
        FloatBuffer i1ColBuff = createFloatBuffer(I1Col);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (I1Col.length * 4), i1ColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);
        //N
        float NVert[] = new float[]{-1.1f, -0.5f, 0.0f, -1.1f, 0.5f, 0.0f, -0.6f, -0.5f, 0.0f, -0.6f, 0.5f, 0.0f};
        float NCol [] = new float[]{GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B,GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B};
        GLES32.glGenVertexArrays(1, vaoN, 0);
        GLES32.glBindVertexArray(vaoN[0]);
        GLES32.glGenBuffers(1, vboNPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboNPos[0]);
        FloatBuffer nPosBuff = createFloatBuffer(NVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (NVert.length * 4), nPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboNCol, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboNCol[0]);
        FloatBuffer nColBuff = createFloatBuffer(NCol);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (NCol.length * 4), nColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //D
        //D ARC
        float dArcVert[] = new float[945];
        float dArcCol[] = new float[945];
        float R = GREEN_R, G = GREEN_G, B = GREEN_B; 
        int i , j;
        float angle = 3 * (float)Math.PI/2;
        for(i=0; i < 315; i++) {
            for(j = 0; j < 3; j++) {
                if(j == 0)
                    dArcVert[(i*3)+j] = -0.1f+(float)(Math.cos(angle))*0.5f;
                else if(j == 1) 
                    dArcVert[(i*3)+j] = (float)Math.sin(angle)*0.5f;
                else 
                    dArcVert[(i*3) +j] = 0.0f;
                
                dArcCol[(i*3)+0] = R;
                dArcCol[(i*3)+1] = G;
                dArcCol[(i*3)+2] = B;
            }
            G += 0.00021f;
            if(G == SAFFRON_G){
                G = SAFFRON_G;
            }
            R += 0.0029f;
            if( R == SAFFRON_R){
                R = SAFFRON_R;
            }
            B += 0.00054f;
            if(B == SAFFRON_B) {
                B = SAFFRON_B;
            }
            angle += 0.01f;
        }
        GLES32.glGenVertexArrays(1, vaoDArc, 0);
        GLES32.glBindVertexArray(vaoDArc[0]);
        GLES32.glGenBuffers(1, vboDArcPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboDArcPos[0]);
        FloatBuffer dAPosBuff = createFloatBuffer(dArcVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (dArcVert.length * 4), dAPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboDArcCol, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboDArcCol[0]);
        FloatBuffer dAColBuff = createFloatBuffer(dArcCol);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (dArcCol.length * 4), dAColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);
        //DLINE
        float dLineVert[] = new float[]{-0.1f, 0.5f, 0.0f, -0.21f, 0.5f, 0.0f, -0.2f, 0.5f, 0.0f, -0.2f, -0.5f, 0.0f, -0.21f, -0.5f, 0.0f, -0.1f, -0.5f, 0.0f};
        float dLineCol[] = new float[]{SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B};
        GLES32.glGenVertexArrays(1, vaoD, 0);
        GLES32.glBindVertexArray(vaoD[0]);
        GLES32.glGenBuffers(1, vboDPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboDPos[0]);
        FloatBuffer dPosBuff = createFloatBuffer(dLineVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (dLineVert.length * 4), dPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboDCol, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboDCol[0]);
        FloatBuffer dColBuff = createFloatBuffer(dLineCol);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (dLineCol.length * 4), dColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //I2
        float I2Vert[] = new float[]{0.7f,0.5f,0.0f,0.7f,-0.5f,0.0f};
        GLES32.glGenVertexArrays(1, vaoI2, 0);
        GLES32.glBindVertexArray(vaoI2[0]);
        GLES32.glGenBuffers(1, vboI2Pos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboI2Pos[0]);
        FloatBuffer i2PosBuff = createFloatBuffer(I2Vert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (I2Vert.length * 4), i2PosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT,false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboI2Col, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboI2Col[0]);
        FloatBuffer i2ColBuff = createFloatBuffer(I1Col);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (I1Col.length * 4), i2ColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        //A
        float aVert[] = new float[]{1.0f, -0.5f, 0.0f, 1.3f, 0.5f, 0.0f, 1.6f, -0.5f, 0.0f, 1.15f,0.025f,0.0f, 1.45f,0.025f,0.0f, 1.15f,0.0f,0.0f,1.45f,0.0f,0.0f, 1.15f,-0.02f,0.0f,1.45f,-0.02f,0.0f};
        float aCol[] = new float[]{GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, GREEN_R,GREEN_G,GREEN_B,GREEN_R,GREEN_G,GREEN_B};
        GLES32.glGenVertexArrays(1, vaoA, 0);
        GLES32.glBindVertexArray(vaoA[0]);
        GLES32.glGenBuffers(1, vboAPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboAPos[0]);
        FloatBuffer aPosBuff = createFloatBuffer(aVert);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (aVert.length * 4), aPosBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glGenBuffers(1, vboACol, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboACol[0]);
        FloatBuffer aColBuff = createFloatBuffer(aCol);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (aCol.length * 4), aColBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glEnable(GLES32.GL_BLEND);
	    GLES32.glBlendFunc(GLES32.GL_SRC_ALPHA, GLES32.GL_ONE_MINUS_SRC_ALPHA);
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

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -3.0f);

        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0); 
        Matrix.multiplyMM(modelViewProjMat, 0 , perspectiveProjMat, 0, modelViewMat, 0);
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjMat, 0);
        GLES32.glLineWidth(9.0f);

        //Draw I
        GLES32.glBindVertexArray(vaoI1[0]);
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glBindVertexArray(0);

        //Draw N
        GLES32.glBindVertexArray(vaoN[0]);
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 1, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 2, 2);
        GLES32.glBindVertexArray(0);

        //D
        GLES32.glBindVertexArray(vaoD[0]);
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 2, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 4, 2);
        GLES32.glBindVertexArray(0);
        GLES32.glBindVertexArray(vaoDArc[0]);
        GLES32.glDrawArrays(GLES32.GL_POINTS, 0, 945);
        GLES32.glBindVertexArray(0);
        //Draw I
        GLES32.glBindVertexArray(vaoI2[0]);
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glBindVertexArray(0);
        //Draw  A
        GLES32.glBindVertexArray(vaoA[0]);
        //Middle Strip
        GLES32.glDrawArrays(GLES32.GL_LINES, 3, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 5, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 7, 2);
        //Lines of A
        GLES32.glDrawArrays(GLES32.GL_LINES, 0, 2);
        GLES32.glDrawArrays(GLES32.GL_LINES, 1, 2);
        GLES32.glBindVertexArray(0);
    

        GLES32.glUseProgram(0);

        requestRender();
    }

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");
        
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

