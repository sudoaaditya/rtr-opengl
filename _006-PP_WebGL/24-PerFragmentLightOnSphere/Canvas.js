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

var sphereObj = null;
//UniformS!.
var modelUniform;
var viewUniform;
var projUnifrom;
var laUniform;
var ldUniform;
var lsUniform;
var lightPosUniform;
var kaUniform;
var kdUniform;
var ksUniform;
var matShineUniform;
var lPressedUnifrom;


//Light Parameters!.
var lightAmbient = [0.1, 0.1, 0.1];
var lightDiffuse = [0.5, 0.2, 0.7];
var lightSpecular = [0.7, 0.0, 0.0];
var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [1.0, 1.0, 1.0];
var materialSpecular = [1.0, 1.0, 1.0];
var materialShininess = 128.0;
var lightPosition = [100.0, 100.0, 100.0, 1.0];
var bLighting = false;

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
    laUniform = gl.getUniformLocation(shaderProgObj, "u_la");
    ldUniform = gl.getUniformLocation(shaderProgObj, "u_ld");
    lsUniform = gl.getUniformLocation(shaderProgObj, "u_ls");
    kaUniform = gl.getUniformLocation(shaderProgObj, "u_ka");
    kdUniform = gl.getUniformLocation(shaderProgObj, "u_kd");
    ksUniform = gl.getUniformLocation(shaderProgObj, "u_ks");
    matShineUniform = gl.getUniformLocation(shaderProgObj, "u_matShine");
    lPressedUnifrom = gl.getUniformLocation(shaderProgObj, "u_lPressed");
    lightPosUniform = gl.getUniformLocation(shaderProgObj, "u_lightPos");

    //Generating Sphere
    sphereObj = new Mesh();
    makeSphere(sphereObj, 2.0, 70, 70);
    //console.log(sphereObj);
    
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
    mat4.translate(modelMat, modelMat, [0.0, 0.0, -5.0]);

    gl.uniformMatrix4fv(modelUniform, false, modelMat);
    gl.uniformMatrix4fv(viewUniform, false, viewMat);
    gl.uniformMatrix4fv(projUnifrom, false, perspectiveProjMat);

    if(bLighting == true) {
        gl.uniform3fv(laUniform, lightAmbient);
        gl.uniform3fv(ldUniform, lightDiffuse);
        gl.uniform3fv(lsUniform, lightSpecular);
        gl.uniform3fv(kaUniform, materialAmbient);
        gl.uniform3fv(kdUniform, materialDiffuse);
        gl.uniform3fv(ksUniform, materialSpecular);
        gl.uniform1f(matShineUniform, materialShininess);
        gl.uniform4fv(lightPosUniform,  lightPosition);
        gl.uniform1i(lPressedUnifrom, 1);
    }
    else {
        gl.uniform1i(lPressedUnifrom, 0);
    }

    sphereObj.draw();

    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
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

