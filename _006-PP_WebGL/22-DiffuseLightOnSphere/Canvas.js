//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;

var sphereObj = null;

var mvUniform;
var projMatUniform;
var kdUniform, ldUniform, lightPosUniform;
var lPressedUniform;
var perspectiveProjMat;

var bLighting = false;
var lightPos = [100.0, 100.0, 100.0, 1.0];

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
            "in vec4 vPosition;" +
            "in vec3 vNormal;" +
            "uniform mat4 u_mvMatrix;" +
            "uniform mat4 u_projMatrix;" +
            "uniform vec3 u_ld;" +
            "uniform vec3 u_kd;" +
            "uniform mediump int u_lKeyPressed;" +
            "uniform vec4 u_lightPosition;" +
            "out vec3 diffuseColor;" +
            "void main(void) {" +
            "if(u_lKeyPressed == 1) {" +
            "   vec4 eyeCoords = u_mvMatrix * vPosition;" +
            "   mat3 normMatrix = mat3(transpose(inverse(u_mvMatrix)));" +
            "   vec3 tNorm = normalize(normMatrix * vNormal);" +
            "   vec3 s = normalize(vec3(u_lightPosition - eyeCoords));" +
            "   diffuseColor = u_ld * u_kd * max(dot(s, tNorm), 0.0);" +
            "}" +
            "gl_Position = u_projMatrix * u_mvMatrix * vPosition;" +
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
            "precision highp float;" +
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

    mvUniform = gl.getUniformLocation(shaderProgObj, "u_mvMatrix");
    projMatUniform = gl.getUniformLocation(shaderProgObj, "u_projMatrix");
    ldUniform = gl.getUniformLocation(shaderProgObj, "u_ld");
    kdUniform = gl.getUniformLocation(shaderProgObj, "u_kd");
    lPressedUniform = gl.getUniformLocation(shaderProgObj, "u_lKeyPressed");
    lightPosUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosition");

    //Generating Sphere
    sphereObj = new Mesh();
    makeSphere(sphereObj, 2.0, 50, 50);
    console.log(sphereObj);
    
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

    var modelViewMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -5.0]);

    gl.uniformMatrix4fv(mvUniform, false, modelViewMat)
    gl.uniformMatrix4fv(projMatUniform, false, perspectiveProjMat);

    if(bLighting === true) {
        gl.uniform1i(lPressedUniform, 1);
        gl.uniform3fv(ldUniform, [0.0, 1.0, 1.0]);
        gl.uniform3fv(kdUniform, [0.7, 0.7, 0.7]);
        gl.uniform4fv(lightPosUniform, lightPos);
    }
    else {
        gl.uniform1i(lPressedUniform, 0);
    }

    sphereObj.draw();

    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
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

