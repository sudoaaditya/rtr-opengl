package com.amk_rtr.ogl_24spheres_light;

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
import java.nio.ShortBuffer;
//For OGL Matric Maths
import android.opengl.Matrix;
import java.lang.Math;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener {

    private GestureDetector gestureDetector;
    private final Context context;
    //OGL Paras
    private int vertexShaderObj;
    private int fragmentShaderObj;
    private int shaderProgObj;
    private int[] vao_sphere = new int[1];
    private int[] vbo_spherePos = new int[1];
    private int[] vbo_sphereNormal = new int[1];
    private int[] vbo_sphereEle = new int[1];
    private int numVertices, numElements;

    //Uniforms
    private int modelUniform;
    private int viewUniform;
    private int projUnifrom;
    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int matShineUniform;
    private int lPressedUnifrom;
    private int laUniform;
    private int ldUniform;
    private int lsUniform;
    private int lightPosUniform;
    private float perspectiveProjMat[] = new float[16];
    private float materialProps[][];

    // Animation Variables!.
    private boolean bLighting = false;
    private float fRotAngle = 0.0f;
    private float Radius = 200.0f;
    private int iKeyPressed = 2; // default Z Axis Rotation; two for Z Axis
    private int iDobTaps = 0;
    private int iWinWidth, iWinHeight;

    //Light Parameters!.
    private float lightAmbient[] = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float lightDiffuse[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float lightSpecular[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float lightPosition[] = new float[]{0.0f, 0.0f, 0.0f, 1.0f};
    
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
        iKeyPressed++;
        if(iKeyPressed >= 2){
            iKeyPressed = 0;
        }
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
        iWinHeight = height;
        iWinWidth = width;
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

        Sphere sphere=new Sphere();
        float sphereVertices[]=new float[1146];
        float sphereNormals[]=new float[1146];
        float sphereTextures[]=new float[764];
        short sphereElements[]=new short[2280];
        sphere.getSphereVertexData(sphereVertices, sphereNormals, sphereTextures, sphereElements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

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
                    "uniform vec4 u_lightPos;" +
                    "out vec3 t_normal;"    +
                    "out vec3 viewerVector;" +
                    "out vec3 lightDirection;" +
                    "void main(void) {" +
                    "   if(u_lPressed == 1) {" +
                    "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" +
                    "       t_normal = mat3(u_viewMat * u_modelMat) * vNormal;" +
                    "       lightDirection = vec3(u_lightPos - eyeCoords);" +
                    "       viewerVector = vec3(-eyeCoords.xyz);" +
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
            "uniform vec3 u_la;" +
            "uniform vec3 u_ld;" +
            "uniform vec3 u_ls;" +
            "uniform vec3 u_ka;" +
            "uniform vec3 u_kd;" +
            "uniform vec3 u_ks;" +
            "uniform float u_matShine;" +
            "uniform int u_lPressed;" +
            "out vec4 FragColor;" +
            "in vec3 t_normal;" +
            "in vec3 lightDirection;" +
            "in vec3 viewerVector;" +
            "void main(void) {" +
            "   vec3 phong_ads_light;" +
            "   if(u_lPressed == 1) {" +
            "       vec3 n_tNormal = normalize(t_normal);" +
            "       vec3 n_lightDirection = normalize(lightDirection);" +
            "       vec3 n_viewerVec = normalize(viewerVector);" +
            "       float tn_dot_ld = max(dot(n_lightDirection, n_tNormal), 0.0);" +
            "       vec3 reflectionVector = reflect(-n_lightDirection, n_tNormal);" +
            "       vec3 ambient = u_la * u_ka;" +
            "       vec3 diffuse = u_ld * u_kd * tn_dot_ld;" +
            "       vec3 specular= u_ls * u_ks * pow(max(dot(reflectionVector, n_viewerVec), 0.0), u_matShine);" +
            "       phong_ads_light = ambient + diffuse + specular;" +
            "   }" +
            "   else {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   FragColor = vec4(phong_ads_light, 1.0);" +
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
        laUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_la");
        ldUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ld");
        lsUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ls");
        kaUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_ks");
        matShineUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_matShine");
        lPressedUnifrom = GLES32.glGetUniformLocation(shaderProgObj, "u_lPressed");
        lightPosUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_lightPos");
            
        //Sphere VAO & VBO!
        
        GLES32.glGenVertexArrays(1, vao_sphere, 0);
        GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glGenBuffers(1, vbo_spherePos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_spherePos[0]);
        FloatBuffer posBuff = createFloatBuffer(sphereVertices);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (sphereVertices.length * 4), posBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        
        GLES32.glGenBuffers(1, vbo_sphereNormal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphereNormal[0]);
        FloatBuffer normBuff = createFloatBuffer(sphereNormals);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (sphereNormals.length * 4), normBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glGenBuffers(1, vbo_sphereEle, 0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphereEle[0]);
        //ShortBuffer Gens!.
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphereElements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer eleBuff = byteBuffer.asShortBuffer();
        eleBuff.put(sphereElements);
        eleBuff.position(0);
        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER, (sphereElements.length * 2), eleBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);

        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        materialProps = getMaterialProps();
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
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for CUBE!
        Matrix.setIdentityM(modelMat, 0);
        Matrix.setIdentityM(viewMat, 0);
        Matrix.setIdentityM(translateMat, 0);

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -2.0f);
        Matrix.multiplyMM(modelMat, 0 , modelMat, 0, translateMat, 0);

        GLES32.glUniformMatrix4fv(modelUniform, 1, false, modelMat, 0);
        GLES32.glUniformMatrix4fv(viewUniform, 1, false, viewMat, 0);
        GLES32.glUniformMatrix4fv(projUnifrom, 1, false, perspectiveProjMat, 0);
        
        if(bLighting == true) {
            GLES32.glUniform3fv(laUniform, 1, lightAmbient, 0);
            GLES32.glUniform3fv(ldUniform, 1, lightDiffuse, 0);
            GLES32.glUniform3fv(lsUniform, 1, lightSpecular, 0);
            
            pushLightPosition(iKeyPressed);
            
            GLES32.glUniform1i(lPressedUnifrom, 1);
        }
        else {
            GLES32.glUniform1i(lPressedUnifrom, 0);
        }
    
        draw24Spheres();
                GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0); 

        requestRender();
    }

    private void update() {
        fRotAngle += 0.01f;
        if(fRotAngle >= 360.0f) {
            fRotAngle = 0.0f;
        }
    }
    
    void pushLightPosition(int animationAlong) {
    
        float fRadius = 100.0f;
    
        if(animationAlong == 0) {
            //Rotate along X.
            lightPosition[1] = fRadius * (float)Math.sin(fRotAngle);
            lightPosition[2] = fRadius * (float)Math.cos(fRotAngle);
        }
        else if( animationAlong == 1) {
            //Rotate along Y.
            lightPosition[0] = fRadius * (float)Math.sin(fRotAngle);
            lightPosition[2] = fRadius * (float)Math.cos(fRotAngle);
        }
        else if( animationAlong == 2) {
            //Rotate along Z.
            lightPosition[0] = fRadius * (float)Math.sin(fRotAngle);
            lightPosition[1] = fRadius * (float)Math.cos(fRotAngle);
        }
    
        GLES32.glUniform4fv(lightPosUniform, 1, lightPosition, 0);
    }
    
    void draw24Spheres() {

        int windowCenterX = iWinWidth / 2;    //Calculate center to set initial viewport to
        int windowCenterY = iWinHeight / 2;
        int relocatedVPSizeX = iWinWidth / 8;     // Divide Width into equal 6 parts, 4 colums and 2 margins
        int relocatedVPSizeY = iWinHeight / 8;    //Divide Height into equal 8 parts, 6 rows and 2 margins
        //Now, if we are changing the viewing volume, we must chnage aspect ratio in perspective
        Matrix.perspectiveM(perspectiveProjMat, 0, 45.0f, (relocatedVPSizeX/relocatedVPSizeY), 0.1f, 100.0f);
        GLES32.glViewport(windowCenterX, windowCenterY, relocatedVPSizeX, relocatedVPSizeY);
    
        int xTransOffset = relocatedVPSizeX;
        int yTransOffset = relocatedVPSizeY;
        int currentViewportX = windowCenterX - xTransOffset*3; //get x to the left most side of X
        int currentViewportY = (int)(windowCenterY + xTransOffset*0.9); // get y to top most side of Y
        //set viewport there!.
        GLES32.glViewport(currentViewportX, currentViewportY, relocatedVPSizeX, relocatedVPSizeY);
    
        int i = 0, j =0;
    
        for(i=0; i<4; i++) {
            currentViewportX += xTransOffset;
            currentViewportY = (int)(windowCenterY + xTransOffset*0.9);
            GLES32.glViewport(currentViewportX, currentViewportY, relocatedVPSizeX, relocatedVPSizeY);
            for(j=0; j<6; j++) {
                if((i*6+j)<24) {
                    pushMaterialData(i*6+j);
                }
                drawSingleSphere();
                currentViewportY -= yTransOffset;
                GLES32.glViewport(currentViewportX, currentViewportY, relocatedVPSizeX, relocatedVPSizeY);
            }
    
        }
    
    }
    void pushMaterialData(int materialOuterIndex) {
        int materialIndex = materialOuterIndex * 4;
        GLES32.glUniform3fv(kaUniform, 1, materialProps[materialIndex], 0);
        GLES32.glUniform3fv(kdUniform, 1, materialProps[materialIndex + 1], 0);
        GLES32.glUniform3fv(ksUniform, 1, materialProps[materialIndex + 2], 0);
        GLES32.glUniform1f(matShineUniform, materialProps[materialIndex + 3][0]);
        
    }
    
    void drawSingleSphere() {

        GLES32.glBindVertexArray(vao_sphere[0]);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphereEle[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);
        GLES32.glBindVertexArray(0);
    }
    

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");

        if(vbo_spherePos[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_spherePos, 0);
            vbo_spherePos[0] = 0;
        }

        if(vbo_sphereNormal[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_sphereNormal, 0);
            vbo_sphereNormal[0] = 0;
        }

        if(vbo_sphereEle[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_sphereEle, 0);
            vbo_sphereEle[0] = 0;
        }

        if(vao_sphere[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0] = 0;
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

    private float[][] getMaterialProps() {
        float materials[][] = new float[][]{

            // Emerald
            
                { 0.0215f, 0.1745f, 0.0215f, 1.0f },
                { 0.07568f, 0.61424f, 0.07568f, 1.0f },
                { 0.633f, 0.727811f, 0.633f, 1.0f },
                {0.6f * 128},
            //Jade   
                { 0.135f, 0.2225f, 0.1575f, 1.0f },
                { 0.54f, 0.89f, 0.63f, 1.0f },
                { 0.316228f, 0.316228f, 0.316228f, 1.0f },
                {0.1f * 128},
            //Obsidian
                { 0.05375f, 0.05f, 0.06625f, 1.0f },
                { 0.18275f, 0.17f, 0.22525f, 1.0f },
                { 0.332741f, 0.328634f, 0.346435f, 1.0f},
                {0.3f * 128},
            //Pearl
                { 0.25f, 0.20725f, 0.20725f, 1.0f },
                { 1.0f, 0.829f, 0.829f, 1.0f },
                { 0.296648f, 0.296648f, 0.296648f, 1.0f },
                {0.088f * 128},
            //Ruby!
                { 0.1745f, 0.01175f, 0.01175f, 1.0f },
                { 0.61424f, 0.04136f, 0.04236f, 1.0f },
                { 0.727811f, 0.626959f, 0.626959f, 1.0f },
                {0.6f * 128},
            //Torquise!
                { 0.1f, 0.18725f, 0.1745f, 1.0f },
                { 0.396f, 0.74151f, 0.69102f, 1.0f },
                { 0.297254f, 0.30829f, 0.306678f, 1.0f },
                {0.1f * 128},
            //Metals :: Brass!.
                { 0.329412f, 0.223529f, 0.027451f, 1.0f },
                { 0.780392f, 0.568627f, 0.113725f, 1.0f },
                { 0.992157f, 0.941176f, 0.807843f, 1.0f },
                {0.21794872f * 128},
            //Bronze!
                { 0.2125f, 0.1275f, 0.054f, 1.0f },
                { 0.714f, 0.4284f, 0.18144f, 1.0f },
                { 0.393548f, 0.271906f, 0.166721f, 1.0f },
                {0.2f * 128},
            //Chrome
                { 0.25f, 0.25f, 0.25f, 1.0f },
                { 0.4f, 0.4f, 0.4f, 1.0f },
                { 0.774597f, 0.774597f, 0.774597f, 1.0f },
                {0.6f * 128},
            //Copper
                { 0.19125f, 0.0735f, 0.0225f, 1.0f },
                { 0.7038f, 0.27048f, 0.0828f, 1.0f },
                { 0.256777f, 0.137622f, 0.086014f, 1.0f },
                {0.1f * 128},
            //Gold. 
                { 0.24725f, 0.1995f, 0.0745f, 1.0f },
                { 0.75164f, 0.60648f, 0.60648f, 1.0f },
                { 0.628281f, 0.555802f, 0.366065f, 1.0f },
                {0.4f * 128},
            //Silver!
                { 0.19225f, 0.19225f, 0.19225f, 1.0f },
                { 0.50754f, 0.50754f, 0.50754f, 1.0f },
                { 0.508273f, 0.508273f, 0.508273f, 1.0f },
                {0.4f * 128},
            //Plastics:: Black
                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.01f,0.01f, 0.01f, 1.0f },
                { 0.50f, 0.50f, 0.50f, 1.0f},
                {0.35f * 128},
            //Cyan
                { 0.0f, 0.1f, 0.06f, 1.0f },
                { 0.0f, 0.50980392f, 0.50980392f, 1.0f },
                { 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },
                {0.25f * 128},
            //Green!.
                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.1f, 0.35f, 0.1f, 1.0f },
                { 0.45f, 0.55f, 0.45f, 1.0f },
                {0.25f * 128},
            //Red!.
                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.5f, 0.0f, 0.0f, 1.0f },
                { 0.7f, 0.6f, 0.6f, 1.0f },
                {0.25f * 128},
            //White!.
                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.55f, 0.55f, 0.55f, 1.0f },
                { 0.70f, 0.70f, 0.70f, 1.0f },
                {0.25f * 128},
            //Yellow!.
                { 0.0f, 0.0f, 0.0f, 1.0f },
                { 0.5f, 0.5f, 0.0f, 1.0f },
                { 0.60f, 0.60f, 0.50f, 1.0f },
                {0.25f * 128},
            //RUbber :: Black!.
                { 0.02f, 0.02f, 0.02f, 1.0f },
                { 0.01f, 0.01f, 0.01f, 1.0f },
                { 0.4f, 0.4f, 0.4f, 1.0f },
                {0.078125f * 128},
            //Cyan!.
                { 0.0f, 0.05f, 0.05f, 1.0f },
                { 0.4f, 0.5f, 0.5f, 1.0f },
                { 0.04f, 0.7f, 0.7f, 1.0f },
                {0.078125f * 128},
            //Green
                { 0.0f, 0.04f, 0.0f, 1.0f },
                { 0.4f, 0.5f, 0.4f, 1.0f },
                { 0.04f, 0.5f, 0.04f, 1.0f },
                {0.078125f * 128},
            //REd!.
                { 0.05f, 0.0f, 0.0f, 1.0f },
                { 0.5f, 0.4f, 0.4f, 1.0f },
                { 0.7f, 0.04f, 0.04f, 1.0f },
                {0.078125f * 128},
            //White
                { 0.05f, 0.05f, 0.05f, 1.0f },
                { 0.5f, 0.5f, 0.5f, 1.0f },
                { 0.7f, 0.7f, 0.7f, 1.0f },
                {0.078125f * 128},
            //Yellow
                { 0.05f, 0.05f, 0.0f, 1.0f },
                { 0.5f, 0.5f, 0.4f, 1.0f },
                { 0.7f, 0.7f, 0.04f, 1.0f },
                {0.078125f * 128}
        };

        return materials;
    }
}

