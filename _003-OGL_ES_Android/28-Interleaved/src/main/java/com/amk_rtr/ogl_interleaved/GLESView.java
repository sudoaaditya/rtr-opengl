package com.amk_rtr.ogl_interleaved;

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
//For textures!.
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.opengl.GLUtils;  //for texImage2D!

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener {

    private GestureDetector gestureDetector;
    private final Context context;
    //OGL Paras
    private int vertexShaderObj;
    private int fragmentShaderObj;
    private int shaderProgObj;
    private int vaoCube[] = new int[1];
    private int vboCube[] = new int[1];

    private int modelUniform;
    private int viewUniform;
    private int projUnifrom;
    private int laUniform;
    private int ldUniform;
    private int lsUniform;
    private int lightPosUniform;
    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int matShineUniform;
    private int lPressedUnifrom;
    private int samplerUniform;

    private int texMarble[] = new int[1];
    private float perspectiveProjMat[] = new float[16];
    

    // Animation Variables!.
    private float fRotAngle = 0.0f;
    private boolean bLighting = false;
    //Light Parameters!.
    private float lightAmbient[] = new float[]{0.1f, 0.1f, 0.1f, 0.1f};
    private float lightDiffuse[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float lightSpecular[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float materialAmbient[] = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float materialDiffuse[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float materialSpecular[] = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float materialShininess = 128.0f;
    private float lightPosition[] = new float[]{0.0f, 0.0f, 2.0f, 1.0f};


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
        bLighting = !bLighting;
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
            "#version 320 es"+
            "\n"+
            "precision mediump float;" +
            "precision lowp int;" +
            "in vec4 vPosition;"+
            "in vec4 vColor;"  +
            "in vec3 vNormal;"+
            "in vec2 vTexCoord;"+
            "uniform mat4 u_modelMat;"+
            "uniform mat4 u_viewMat;"+
            "uniform mat4 u_projMat;"+
            "uniform int u_lPressed;"+
            "uniform vec4 u_lightPos;"+
            "out vec3 t_normal;"   +
            "out vec2 out_texCoord;"  +
            "out vec4 out_color;"  +
            "out vec3 viewerVector;"+
            "out vec3 lightDirection;" +
            "void main(void) {"+
            "   if(u_lPressed == 1) {"+
            "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;"+
            "       t_normal = mat3(u_viewMat * u_modelMat) * vNormal;"+
            "       lightDirection = vec3(u_lightPos - eyeCoords);"+
            "       viewerVector = vec3(-eyeCoords.xyz);"+
            "   }"+
            "   gl_Position = u_projMat * u_viewMat * u_modelMat * vPosition;" +
            "   out_color = vColor;"   +
            "   out_texCoord = vTexCoord;"   +
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
            "uniform sampler2D u_sampler;"  +
            "uniform float u_matShine;" +
            "uniform int u_lPressed;" +
            "out vec4 FragColor;" +
            "in vec3 t_normal;" +
            "in vec3 lightDirection;" +
            "in vec3 viewerVector;" +
            "in vec2 out_texCoord;"   +
            "in vec4 out_color;"    +
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
            "   vec3 tex = vec3(texture(u_sampler, out_texCoord));"  +
            "   FragColor = vec4((tex * vec3(out_color) * phong_ads_light), 1.0);" +
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
        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
        GLES32.glBindAttribLocation(shaderProgObj, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord");

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
        samplerUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_sampler");

        //Cube Vao & Vbo
        float cubeVCNT[] = new float[]{
            -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 
            1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,

            - 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   
            -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,   
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,1.0f, 0.0f,

            -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,1.0f, 1.0f, 
            -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,   
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,0.0f, 0.0f,   
            1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,1.0f, 0.0f,
        };

        GLES32.glGenVertexArrays(1, vaoCube, 0);
        GLES32.glBindVertexArray(vaoCube[0]);

        GLES32.glGenBuffers(1, vboCube, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vboCube[0]);
        FloatBuffer vcntBuff = createFloatBuffer(cubeVCNT);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (cubeVCNT.length * 4), vcntBuff, GLES32.GL_STATIC_DRAW);
        
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, (11 * 4), (0 * 4));
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR, 3, GLES32.GL_FLOAT, false, (11 * 4), (3 * 4));
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL, 3, GLES32.GL_FLOAT, false, (11 * 4), (6 * 4));
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, (11 * 4), (9 * 4));
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        GLES32.glBindVertexArray(0);

        texMarble[0] = loadTexture(R.raw.marble);
        if(texMarble[0] == 0) {
            System.out.println("RTR: Faled To Load Texture");
            uninitialize();
            System.exit(0);
        }
        else {
            System.out.println("RTR: Texture Smiley Loaded Successfully!!");
        }

        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        Matrix.setIdentityM(perspectiveProjMat, 0);
    
    }

    private int loadTexture(int imgResID) {
        int texture[] = new int[1];
        BitmapFactory.Options options = new BitmapFactory.Options();

        options.inScaled = false;
        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), imgResID, options);

        GLES32.glGenTextures(1, texture, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture[0]);
        GLES32.glPixelStorei(GLES32.GL_TEXTURE_2D, 1);

        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);

        //Now create texture.
        GLUtils.texImage2D(GLES32.GL_TEXTURE_2D, 0, bitmap, 0);
        GLES32.glGenerateMipmap(GLES32.GL_TEXTURE_2D);

        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);

        return(texture[0]);
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
        float scaleMat[] = new float[16];
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for CUBE!
        Matrix.setIdentityM(modelMat, 0);
        Matrix.setIdentityM(viewMat, 0);
        Matrix.setIdentityM(translateMat, 0);
        Matrix.setIdentityM(rotateMat, 0);
        Matrix.setIdentityM(scaleMat, 0);

        Matrix.translateM(translateMat, 0, 0.0f, 0.0f, -5.0f);
        Matrix.multiplyMM(modelMat, 0 , modelMat, 0, translateMat, 0);

        Matrix.scaleM(scaleMat, 0, 0.75f, 0.75f, 0.75f);
        Matrix.multiplyMM(modelMat, 0, modelMat, 0, scaleMat, 0);
        
        Matrix.setRotateM(rotateMat, 0, fRotAngle, 1.0f, 1.0f, 1.0f);
        Matrix.multiplyMM(modelMat, 0, modelMat, 0, rotateMat, 0);

        GLES32.glUniformMatrix4fv(modelUniform, 1, false, modelMat, 0);
        GLES32.glUniformMatrix4fv(viewUniform, 1, false, viewMat, 0);
        GLES32.glUniformMatrix4fv(projUnifrom, 1, false, perspectiveProjMat, 0);

        if(bLighting == true) {
            GLES32.glUniform3fv(laUniform, 1, lightAmbient, 0);
            GLES32.glUniform3fv(ldUniform, 1, lightDiffuse, 0);
            GLES32.glUniform3fv(lsUniform, 1, lightSpecular, 0);
            GLES32.glUniform3fv(kaUniform, 1, materialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, materialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, materialSpecular, 0);
            GLES32.glUniform1f(matShineUniform, materialShininess);
            GLES32.glUniform4fv(lightPosUniform, 1, lightPosition, 0);
            GLES32.glUniform1i(lPressedUnifrom, 1);
        }
        else {
            GLES32.glUniform1i(lPressedUnifrom, 0);
        }        

        GLES32.glBindVertexArray(vaoCube[0]);
        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texMarble[0]);
        GLES32.glUniform1i(samplerUniform, 0);

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
