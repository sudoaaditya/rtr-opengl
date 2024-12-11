package com.amk_rtr.ogl_3dtexture;

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
    
    private int vao_pyramid[] = new int[1];
    private int vbo_pyramidPos[] = new int[1];
    private int vbo_pyramidTex[] = new int[1];

    private int vao_cube[] = new int[1];
    private int vbo_cubePos[] = new int[1];
    private int vbo_cubeTex[] = new int[1];

    private int samplerUniform;
    private int mvpUniform;
    private float perspectiveProjMat[] = new float[16];
    private int texStone[] = new int[1];
    private int texKundali[] = new int[1];
    
    private float fPyramidRotAngle = 0.0f;
    private float fCubeRotAngle = 0.0f;

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
            "in vec4 vPosition;" +
            "in vec2 vTexCoord;" +
            "out vec2 out_texCoord;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void)" +
            "{" +
            "gl_Position = u_mvp_matrix * vPosition;" +
            "out_texCoord = vTexCoord;" +
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
            "out vec4 FragColor;" +
            "in vec2 out_texCoord;" +
            "uniform sampler2D u_sampler;" +
            "void  main(void) {" +
            "FragColor = texture(u_sampler, out_texCoord);" +
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
        
        mvpUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_mvp_matrix");
        samplerUniform = GLES32.glGetUniformLocation(shaderProgObj, "u_sampler");

        //For Pyramid
        final float pyramidVert[] = new float[] {
            0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
            0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
            0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f
        };

        final float pyramidTexCoords[] =  new float[]{ 
            0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f 
        };

        GLES32.glGenVertexArrays(1, vao_pyramid, 0);
        GLES32.glBindVertexArray(vao_pyramid[0]);

        GLES32.glGenBuffers(1, vbo_pyramidPos, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramidPos[0]);
        //Create Buffer that is eligible to sent oer GLBufferData
        FloatBuffer posBuffer = createFloatBuffer(pyramidVert);
        //glBuffdata
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (pyramidVert.length * 4), posBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glGenBuffers(1, vbo_pyramidTex, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_pyramidTex[0]);
        FloatBuffer texBuffer = createFloatBuffer(pyramidTexCoords);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (pyramidTexCoords.length *4), texBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);


        //For Cube
        final float cubeVert[] = new float[]{
            1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f
        };

        final float cubeTexCoord[] = new float[] {
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
            
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
        
        GLES32.glGenBuffers(1, vbo_cubeTex, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_cubeTex[0]);
        FloatBuffer texCBuff = createFloatBuffer(cubeTexCoord);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, (cubeTexCoord.length * 4), texCBuff, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, 0, 0);
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);


        texStone[0] = loadTexture(R.raw.stone);
        if(texStone[0] == 0) {
            System.out.println("RTR: Failed To Load Stone Texture");
            uninitialize();
            System.exit(0);
        }
        else {
            System.out.println("RTR: Texture Stone Loaded Successfully!!");
        }

        texKundali[0] = loadTexture(R.raw.kundali);
        if(texKundali[0] == 0) {
            System.out.println("RTR: Failed To Load Kundali Texture");
            uninitialize();
            System.exit(0);
        }
        else {
            System.out.println("RTR: Texture Kundali Loaded Successfully!!");
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
        float modelViewMat[] = new float[16];
        float modelViewProjMat[] = new float[16];
        float translateMat[] = new float[16];
        float rotateMat[] = new float[16];
        float scaleMat[] = new float[16];
        
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glUseProgram(shaderProgObj);

        //Things we do for Pyramid!
        Matrix.setIdentityM(modelViewProjMat, 0);
        Matrix.setIdentityM(modelViewMat, 0);
        Matrix.setIdentityM(translateMat, 0);
        Matrix.setIdentityM(rotateMat, 0);

        Matrix.translateM(translateMat, 0, -2.0f, 0.0f, -6.0f);
        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0); 

        Matrix.setRotateM(rotateMat, 0, fPyramidRotAngle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelViewMat, 0, modelViewMat, 0, rotateMat, 0);

        Matrix.multiplyMM(modelViewProjMat, 0 , perspectiveProjMat, 0, modelViewMat, 0);
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjMat, 0);

        //ABU for textute
        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texStone[0]);
        GLES32.glUniform1i(samplerUniform, 0);

        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);

        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, 0);
        GLES32.glBindVertexArray(0);

        //Things we do for CUBE
        Matrix.setIdentityM(modelViewProjMat, 0);
        Matrix.setIdentityM(modelViewMat, 0);
        Matrix.setIdentityM(translateMat, 0);
        Matrix.setIdentityM(rotateMat, 0);
        Matrix.setIdentityM(scaleMat, 0);

        Matrix.translateM(translateMat, 0, 2.0f, 0.0f, -6.0f);
        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, translateMat, 0); 

        Matrix.scaleM(scaleMat, 0, 0.75f, 0.75f, 0.75f);
        Matrix.multiplyMM(modelViewMat, 0 , modelViewMat, 0, scaleMat, 0); 

        Matrix.setRotateM(rotateMat, 0, fCubeRotAngle, 1.0f, 1.0f, 1.0f);
        Matrix.multiplyMM(modelViewMat, 0, modelViewMat, 0, rotateMat, 0);
        
        Matrix.multiplyMM(modelViewProjMat, 0 , perspectiveProjMat, 0, modelViewMat, 0);
        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjMat, 0);

        GLES32.glActiveTexture(GLES32.GL_TEXTURE0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texKundali[0]);
        GLES32.glUniform1i(samplerUniform, 0);

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

        fPyramidRotAngle += 0.5f;
        if(fPyramidRotAngle >= 360.0f) {
            fPyramidRotAngle = 0.0f;
        }

        fCubeRotAngle += 0.5f;
        if(fCubeRotAngle >= 360.0f) {
            fCubeRotAngle = 0.0f;
        }
    }

    private void uninitialize() {
        System.out.println("RTR: Clled UnInit");
        if(vbo_pyramidPos[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_pyramidPos, 0);
            vbo_pyramidPos[0] = 0;
        }

        if(vbo_pyramidTex[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_pyramidTex, 0);
            vbo_pyramidTex[0] = 0;
        }

        if(vao_pyramid[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_pyramid, 0);
            vao_pyramid[0] = 0;
        }

        if(vbo_cubePos[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_cubePos, 0);
            vbo_cubePos[0] = 0;
        }

        if(vbo_cubeTex[0] != 0) {
            GLES32.glDeleteBuffers(1, vbo_cubeTex, 0);
            vbo_cubeTex[0] = 0;
        }

        if(vao_cube[0] != 0) {
            GLES32.glDeleteVertexArrays(1, vao_cube, 0);
            vao_cube[0] = 0;
        }

        if(texStone[0] != 0) {
            GLES32.glDeleteTextures(1, texStone, 0);
            texStone[0] = 0;
        }

        if(texKundali[0] != 0) {
            GLES32.glDeleteTextures(1, texKundali, 0);
            texKundali[0] = 0;
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

