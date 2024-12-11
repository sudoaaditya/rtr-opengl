//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;

var vaoPyramid;
var vboPyramidPos, vboPyramidNorm;

//UniformS!.
var modelUniform;
var viewUniform;
var projUnifrom;
var lightPosUniform;
var kaUniform;
var kdUniform;
var ksUniform;
var matShineUniform;
var lPressedUnifrom;

var laOneUniform;
var ldOneUniform;
var lsOneUniform;
var lightPosOneUniform;

var laTwoUniform;
var ldTwoUniform;
var lsTwoUniform;
var lightPosTwoUniform;

//Animation Vars!.
var fRotAngle = 0.0;
var bLighting = false;

//Light Parameters!.
//Light Parameters Right Pars:: Red Light!.
var lightAmbientRed = [0.0, 0.0, 0.0];
var lightDiffuseRed = [1.0, 0.0, 0.0];
var lightSpecularRed = [1.0, 0.0, 0.0];
var lightPositionRed = [2.0, 0.0, 0.0, 1.0];
//Light For Left Pars:: Blue Light
var lightAmbientBlue = [0.0, 0.0, 0.0];
var lightDiffuseBlue = [0.0, 0.0, 1.0];
var lightSpecularBlue = [0.0, 0.0, 1.0];
var lightPositionBlue = [-2.0, 0.0, 0.0, 1.0];

var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [1.0, 1.0, 1.0];
var materialSpecular = [1.0, 1.0, 1.0];
var materialShininess = 128.0;
var lightPosition = [100.0, 100.0, 100.0, 1.0];


//Request Animation Frame!
var requestAnimationFrame = 
    window.requestAnimationFrame ||
	window.webkitRequestAnimationFrame ||
	window.mozRequestAnimationFrame ||
	window.oRequestAnimationFrame ||
	window.msRequestAnimationFrame ||
	null;

const main = function() {
    //Acquire canvas!
    canvas = document.getElementById("AMC");
    if(!canvas){
        console.log("Failed To Acquire Canvas Element!..");
    }
    else {
        console.log("Canvas Element Acquired Successfully!..");
    }

    canvasOriginalWidth = canvas.width;
    canvasOriginalHeight = canvas.height;

    //Add Event Listener!.
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseEvent, false);
    window.addEventListener("resize", resize, false);

    //Init.
    initialize();
    resize();
    display();
}

const toggleFullScreen = function(){
    //code/
    var fullScreenEelemet = document.fullscreenElement || document.webkitFullscrenElement ||
                            document.mozFullScreenElement || document.msFullscreenElement || null;

    if(fullScreenEelemet == null) {
        if(canvas.requestFullscreen){
            canvas.requestFullscreen();
        }
        else if(canvas.mozRequestFullScreen) {
            canvas.mozRequestFullScreen();
        }
        else if(canvas.webkitRequestFullscreen) {
            canvas.webkitRequestFullscreen();
        }
        else if(canvas.msRequestFullscreen) {
            canvas.msRequestFullscreen();
        }

        bFullScreen = true;
    }
    else {
        if(document.exitFullscreen) {
            document.exitFullscreen();
        }
        else if(document.mozCancelFullScreen) {
            document.mozCancelFullScreen();
        }
        else if(document.webkitExitFullscreen) {
            document.webkitExitFullscreen();
        }
        else if(document.msExitFullscreen) {
            document.msExitFullscreen();
        }
        bFullScreen = false;
    }
}

const initialize = function() {
    //Code!.
    gl = canvas.getContext("webgl2");
    if(gl == null){
        console.log("Failed to get WebGL2 Context!!");
    }
    else {
        console.log("Successfully Acquired the WebGL2 Context!!");
    }

    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //Vertex Shader; 
    var vertexShaderSource = 
            "#version 300 es" +     //OpenGL ES 300 vaprun 2.0 WbeGL SO!
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
            "}";
    
    vertexShaderObj = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObj, vertexShaderSource);
    gl.compileShader(vertexShaderObj);
    if(gl.getShaderParameter(vertexShaderObj, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(vertexShaderObj);
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Vertex Shader Compiled Successfully!");
    }

    var fragmentShaderSource = 
            "#version 300 es" +
            "\n" +
            "precision mediump float;" +
            "precision lowp int;" +
            "out vec4 FragColor;" +
            "in vec3 phong_ads_light;"+
            "void main(void)" +
            "{" +
            "FragColor = vec4(phong_ads_light, 1.0);" +
            "}";

    fragmentShaderObj = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObj, fragmentShaderSource);
    gl.compileShader(fragmentShaderObj);
    if(gl.getShaderParameter(fragmentShaderObj, gl.COMPILE_STATUS) == false){
        var error = gl.getShaderInfoLog(fragmentShaderObj);
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Fragment Shader Compiled Successfully!");
    }

    //Shader Prog!.
    shaderProgObj = gl.createProgram();
    gl.attachShader(shaderProgObj, vertexShaderObj);
    gl.attachShader(shaderProgObj, fragmentShaderObj);

    //Pre-Link!
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    gl.linkProgram(shaderProgObj);
    if(gl.getProgramParameter(shaderProgObj, gl.LINK_STATUS) == false) {
        var error = gl.getProgramInfoLog(shaderProgObj)
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Shader Program Linked Successfully!");
    }

    modelUniform = gl.getUniformLocation(shaderProgObj, "u_modelMat");
    viewUniform = gl.getUniformLocation(shaderProgObj, "u_viewMat");
    projUnifrom = gl.getUniformLocation(shaderProgObj, "u_projMat");
    laOneUniform = gl.getUniformLocation(shaderProgObj, "u_laOne");
    ldOneUniform = gl.getUniformLocation(shaderProgObj, "u_ldOne");
    lsOneUniform = gl.getUniformLocation(shaderProgObj, "u_ls");
    lightPosOneUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosOne");

    laTwoUniform = gl.getUniformLocation(shaderProgObj, "u_laTwo");
    ldTwoUniform = gl.getUniformLocation(shaderProgObj, "u_ldTwo");
    lsTwoUniform = gl.getUniformLocation(shaderProgObj, "u_lsTwo");
    lightPosTwoUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosTwo");
    
    kaUniform = gl.getUniformLocation(shaderProgObj, "u_ka");
    kdUniform = gl.getUniformLocation(shaderProgObj, "u_kd");
    ksUniform = gl.getUniformLocation(shaderProgObj, "u_ks");
    matShineUniform = gl.getUniformLocation(shaderProgObj, "u_matShine");
    lPressedUnifrom = gl.getUniformLocation(shaderProgObj, "u_lPressed");
    
    //PyramidThings;
    var pyramidVertices = new Float32Array([
        0.0, 1.0, 0.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0,
        0.0, 1.0, 0.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0,
        0.0, 1.0, 0.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0,
        0.0, 1.0, 0.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0
    ]);

    var pyramidNormals = new Float32Array([
        0.0, 0.447214, 0.894427, 0.0, 0.447214, 0.894427, 0.0, 0.447214, 0.894427, 
        0.894427, 0.447214, 0.0, 0.894427, 0.447214, 0.0, 0.894427, 0.447214, 0.0, 
        0.0, 0.447214, -0.894427, 0.0, 0.447214, -0.894427, 0.0, 0.447214, -0.894427, 
        -0.894427, 0.447214, 0.0, -0.894427, 0.447214, 0.0, -0.894427, 0.447214, 0.0
    ])

    vaoPyramid = gl.createVertexArray();
    gl.bindVertexArray(vaoPyramid);
    vboPyramidPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPyramidPos);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboPyramidNorm = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPyramidNorm);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    //gl.enable(gl.CULL_FACE);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjMat = mat4.create();

}

const resize = function() {
    if(bFullScreen == true){
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else {
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    mat4.perspective(perspectiveProjMat, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0);
}

const display = function() {
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelMat = mat4.create();
    var viewMat = mat4.create();
    mat4.translate(modelMat, modelMat, [0.0, 0.0, -4.0]);
    mat4.rotateY(modelMat, modelMat, degToRad(fRotAngle));

    gl.uniformMatrix4fv(modelUniform, false, modelMat);
    gl.uniformMatrix4fv(viewUniform, false, viewMat);
    gl.uniformMatrix4fv(projUnifrom, false, perspectiveProjMat);

    if(bLighting == true) {
        gl.uniform3fv(laOneUniform, lightAmbientRed);
        gl.uniform3fv(ldOneUniform, lightDiffuseRed);
        gl.uniform3fv(lsOneUniform, lightSpecularRed);
        gl.uniform4fv(lightPosOneUniform, lightPositionRed);
    
        gl.uniform3fv(laTwoUniform, lightAmbientBlue);
        gl.uniform3fv(ldTwoUniform, lightDiffuseBlue);
        gl.uniform3fv(lsTwoUniform, lightSpecularRed);
        gl.uniform4fv(lightPosTwoUniform, lightPositionBlue);
    
        gl.uniform3fv(kaUniform, materialAmbient);
        gl.uniform3fv(kdUniform, materialDiffuse);
        gl.uniform3fv(ksUniform, materialSpecular);
        gl.uniform1f(matShineUniform, materialShininess);
        gl.uniform1i(lPressedUnifrom, 1);
    }
    else {
        gl.uniform1i(lPressedUnifrom, 0);
    }

    gl.bindVertexArray(vaoPyramid);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);

    gl.useProgram(null);
    update();
    requestAnimationFrame(display, canvas);
}

const update = function() {
    fRotAngle += 0.5;
    if(fRotAngle >= 360.0){
        fRotAngle = 0.0;
    }
}

const degToRad = function(degAngle) {
    return( degAngle * Math.PI / 180);
}

const uninitialize = function() {
    //Code!.
    if(sphereObj) {
        sphereObj.deallocate();
        sphereObj = null;
    }

    if(shaderProgObj) {
        if(fragmentShaderObj) {
            gl.detachShader(shaderProgObj, fragmentShaderObj);
            gl.deleteShader(fragmentShaderObj);
            fragmentShaderObj = null;
        }

        if(vertexShaderObj) {
            gl.detachShader(shaderProgObj, vertexShaderObj);
            gl.deleteShader(vertexShaderObj);
            vertexShaderObj = null;
        }

        gl.deleteProgram(shaderProgObj);
        shaderProgObj = null;
    }
}

const keyDown = function(event) {
    
    switch(event.keyCode) {
        case 27:
            uninitialize();
            window.close();
            break;

        case 70:
            toggleFullScreen();
            break;

        case 76:
            bLighting = bLighting === true ? false : true;
            break;
    }
}

const mouseEvent = function() {
    //Code!
}

