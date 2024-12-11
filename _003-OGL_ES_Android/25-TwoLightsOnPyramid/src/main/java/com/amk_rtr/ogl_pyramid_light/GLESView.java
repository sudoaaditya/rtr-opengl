package com.amk_rtr.ogl_pyramid_light;

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
    private int[] vao_pyramid = new int[1];
    private int[] vbo_pyramidPos = new int[1];
    private int[] vbo_pyramidNormal = new int[1];

    //Uniforms
    private int modelUniform;
    private int viewUniform;
    private int projUnifrom;
    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int matShineUniform;
    private int lPressedUnifrom;
    
    private int laOneUniform;
    private int ldOneUniform;
    private int lsOneUniform;
    private int lightPosOneUniform;

    private int laTwoUniform;
    private int ldTwoUniform;
    private int lsTwoUniform;
    private int lightPosTwoUniform;

    
    private float perspectiveProjMat[] = new float[16];

    // Animation Variables!.
    private boolean bLighting = false;
    private float fRotAngle = 0.0f;
    //Light Parameters!.
    //Light Parameters Right Pars:: Red Light!.
    private float lightAmbientRed[] = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float lightDiffuseRed[] = new float[]{1.0f, 0.0f, 0.0f, 1.0f};
    private float lightSpecularRed[] = new float[]{1.0f, 0.0f, 0.0f, 1.0f};
    private float lightPositionRed[] = new float[]{2.0f, 0.0f, 0.0f, 1.0f};
    //Light For Left Pars:: Blue Light
    private float lightAmbientBlue[] = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float lightDiffuseBlue[] = new float[]{0.0f, 0.0f, 1.0f, 1.0f};
    private float lightSpecularBlue[] = new float[]{0.0f, 0.0f, 1.0f, 1.0f};
    private float lightPositionBlue[] = new float[]{-2.0f, 0.0f, 0.0f, 1.0f};

    private float materialAmbient[] = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float materialDiffuse[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float materialSpecular[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;
    
    
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
        bLighting = bLighting == true ? false :  true;
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
        update();
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
            "uniform mat4 u_modelMat;" +
            "uniform mat4 u_viewMat;" +
            "uniform mat4 u_projMat;" +
            "uniform int u_lPressed;" +
            "uniform vec3 u_laOne;" +
            "uniform vec3 u_ldOne;" +
            "uniform vec3 u_lsOne;" +
            "uniform vec4 u_lightPosOne;" +
            "uniform vec3 u_laTwo;" +
            "uniform vec3 u_ldTwo;" +
            "uniform vec3 u_lsTwo;" +
            "uniform vec4 u_lightPosTwo;" +
            "uniform vec3 u_ka;" +
            "uniform vec3 u_kd;" +
            "uniform vec3 u_ks;" +
            "uniform float u_matShine;" +
            "out vec3 phong_ads_light;" +
            "void main(void) {" +
            "   if(u_lPressed == 1) {" +
            "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" +
            "       vec3 t_normal = normalize(mat3(u_viewMat * u_modelMat) * vNormal);" +

            "       vec3 lightDirectionOne = normalize(vec3(u_lightPosOne - eyeCoords));" +
            "       float tn_dot_ldOne = max(dot(lightDirectionOne, t_normal), 0.0);" +
            "       vec3 reflectionVectorOne = reflect(-lightDirectionOne, t_normal);" +

            "       vec3 lightDirectionTwo = normalize(vec3(u_lightPosTwo - eyeCoords));" +
            "       float tn_dot_ldTwo = max(dot(lightDirectionTwo, t_normal), 0.0);" +
            "       vec3 reflectionVectorTwo = reflect(-lightDirectionTwo, t_normal);" +

            "       vec3 viewerVector = normalize(vec3(-eyeCoords.xyz));" +

            "       vec3 ambientOne = u_laOne * u_ka;" +
            "       vec3 diffuseOne = u_ldOne * u_kd * tn_dot_ldOne;" +
            "       vec3 specularOne = u_lsOne * u_ks * pow(max(dot(reflectionVectorOne, viewerVector), 0.0), u_matShine);" +

            "       vec3 ambientTwo = u_laTwo * u_ka;" +
            "       vec3 diffuseTwo = u_ldTwo * u_kd * tn_dot_ldTwo;" +
            "       vec3 specularTwo = u_lsTwo * u_ks * pow(max(dot(reflectionVectorTwo, viewerVector), 0.0), u_matShine);" +

            "       phong_ads_light = ambientOne + ambientTwo +diffuseOne + diffuseTwo +specularOne + specularTwo;" +
            "   }" +
            "   else {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   gl_Position = u_projMat * u_viewMat * u_modelMat * vPosition;"  +
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
            "in vec3 phong_ads_light;"+
            "void main(void)" +
            "{" +
            "FragColor = vec4(phong_ads_light, 1.0);" +
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

        modelUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_modelMat");
        viewUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_viewMat");
        projUnifrom = GLES32.glGetUniformLocation(shaderProgObj, "u_projMat");
        laOneUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_laOne");
        ldOneUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ldOne");
        lsOneUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ls");
        lightPosOneUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lightPosOne");

        laTwoUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_laTwo");
        ldTwoUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ldTwo");
        lsTwoUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lsTwo");
        lightPosTwoUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lightPosTwo");
        
        kaUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ks");
        matShineUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_matShine");
        lPressedUnifrom = GLES32.glGetUniformLocation(shaderProgObj, "u_lPressed");
        
        
        //Pyramid VAO & VBO!
        final float pyramidVertices[] = new float[]{ 
            0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
            0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
            0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f
        };

        final float pyramidNormals[] = new float[]{
            0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f, 
            0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f, 
            0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f, 
            -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f
        };
        
        GLES32.glGenVertexArrays(1, vao_pyramid, 0);
        GLES32.glBindVertexArray(vao_pyramid[0]);

        GLES32.glGenBuffers(1, vbo_pyramidPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramidPos[0]);
        FloatBuffer posBuff = createFloatBuffer(pyramidVertices);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (pyramidVertices.length * 4), posBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
        GLES32.glGenBuffers(1, vbo_pyramidNormal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramidNormal[0]);
        FloatBuffer normBuff = createFloatBuffer(pyramidNormals);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (pyramidNormals.length * 4), normBuff, GLES32.GL_STATIC_DRAW);
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
        float modelMat[] = new float[16];
        float viewMat[] = new float[16];
        float translateMat[] = new float[16];
        float rotateMat[] = new float[16];
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for CUBE!
        Matrix.setIdentityM(modelMat, 0);
        Matrix.setIdentityM(viewMat, 0);
        Matrix.setIdentityM(translateMat, 0);
        Matrix.setIdentityM(rotateMat, 0);

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -5.0f);
        Matrix.multiplyMM(modelMat, 0 , modelMat, 0, translateMat, 0);

        Matrix.setRotateM(rotateMat, 0, fRotAngle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelMat, 0, modelMat, 0, rotateMat, 0);

        GLES32.glUniformMatrix4fv(modelUniform, 1, false, modelMat, 0);
        GLES32.glUniformMatrix4fv(viewUniform, 1, false, viewMat, 0);
        GLES32.glUniformMatrix4fv(projUnifrom, 1, false, perspectiveProjMat, 0);

        if(bLighting == true) {
            GLES32.glUniform3fv(laOneUniform, 1, lightAmbientRed, 0);
            GLES32.glUniform3fv(ldOneUniform, 1, lightDiffuseRed, 0);
            GLES32.glUniform3fv(lsOneUniform, 1, lightSpecularRed, 0);
            GLES32.glUniform4fv(lightPosOneUniform, 1, lightPositionRed, 0);
    
            GLES32.glUniform3fv(laTwoUniform, 1, lightAmbientBlue, 0);
            GLES32.glUniform3fv(ldTwoUniform, 1, lightDiffuseBlue, 0);
            GLES32.glUniform3fv(lsTwoUniform, 1, lightSpecularRed, 0);
            GLES32.glUniform4fv(lightPosTwoUniform, 1, lightPositionBlue, 0);
    
            GLES32.glUniform3fv(kaUniform, 1, materialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, materialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, materialSpecular, 0);
            GLES32.glUniform1f(matShineUniform, materialShininess);
            GLES32.glUniform1i(lPressedUnifrom, 1);
        }
        else {
            GLES32.glUniform1i(lPressedUnifrom, 0);
        }
    
        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
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

        if(vbo_pyramidPos[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_pyramidPos, 0);
            vbo_pyramidPos[0] = 0;
        }

        if(vbo_pyramidNormal[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_pyramidNormal, 0);
            vbo_pyramidNormal[0] = 0;
        }

        if(vao_pyramid[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_pyramid, 0);
            vao_pyramid[0] = 0;
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

