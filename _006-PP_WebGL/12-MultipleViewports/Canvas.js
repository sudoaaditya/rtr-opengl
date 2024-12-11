//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

var canvasCurrentWH = {
    width:0,
    height:0
};
var viewportCoords = {
    x:0,
    y:0,
    w:0,
    h:0
};

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;

var vao;
var vbo;
var mvpUniform;
var perspectiveProjMat;


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

    resize(0);
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
        console.log("Failed to get WebGL2 Context!");
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
            "uniform mat4 u_mvp_matrix;" +
            "void main(void) {" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
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
            "void main(void) {" +
            "   FragColor = vec4(0.5, 0.2, 0.7, 1.0);" +
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

    mvpUniform = gl.getUniformLocation(shaderProgObj, "u_mvp_matrix");

    //Vertices!.
    var triangleVertices = new Float32Array([
		0.0,1.0,0.0,
		-1.0,-1.0,0.0,
		1.0,-1.0,0.0
		]);

    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);

    vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjMat = mat4.create();

}

const resize = function(keyVal) {
    if(bFullScreen === true && keyVal === 0){
        viewportCoords.x = 0;
        viewportCoords.y = 0;
        viewportCoords.w = window.innerWidth;
        viewportCoords.h = window.innerHeight;
        canvas.height = window.innerHeight;
        canvas.width = window.innerWidth;
    }
    else if(!bFullScreen && keyVal === 0) {
        viewportCoords.x = 0;
        viewportCoords.y = 0;
        viewportCoords.w = canvasOriginalWidth;
        viewportCoords.h = canvasOriginalHeight;
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }
    console.log(viewportCoords);
    
    gl.viewport(viewportCoords.x, viewportCoords.y, viewportCoords.w, viewportCoords.h);

    mat4.perspective(perspectiveProjMat, 45.0, parseFloat(viewportCoords.w)/parseFloat(viewportCoords.h), 0.1, 100.0);
}

const display = function() {
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    gl.bindVertexArray(vao);
    gl.drawArrays(gl.TRIANGLES, 0, 3);

    gl.bindVertexArray(null);
    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
}


const uninitialize = function() {
    //Code!.

    if(vbo){
        gl.deleteBuffer(vbo);
        vbo = null;
    }
    if(vao) {
        gl.deleteVertexArray(vao);
        vao = null;
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

    canvasCurrentWH.width = canvas.width;
    canvasCurrentWH.height = canvas.height;    
    switch(event.keyCode) {

        case 27:
            uninitialize();
            window.close();
            break;

        case 70:
            toggleFullScreen();
            break;

        case 49:
        case 97:
            viewportCoords={x:0, y:0, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 50:
        case 98:
            viewportCoords={x:canvasCurrentWH.width/2, y:0, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 51:
        case 99:
            viewportCoords={x:0, y:canvasCurrentWH.height/2, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 52:
        case 100:
            viewportCoords={x:canvasCurrentWH.width/2, y:canvasCurrentWH.height/2, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 53:
        case 101:
            viewportCoords={x:0, y:0, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height};
            resize(1);
            break;

        case 54:
        case 102:
            //6
            viewportCoords={x:canvasCurrentWH.width/2, y:0, w:canvasCurrentWH.width/2, h:canvasCurrentWH.height};
            resize(1);
            break;

        case 55:
        case 103:
            //7
            viewportCoords={x:0, y:canvasCurrentWH.height/2, w:canvasCurrentWH.width, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 56:
        case 104:
            //8
            viewportCoords={x:0, y:0, w:canvasCurrentWH.width, h:canvasCurrentWH.height/2};
            resize(1);
            break;

        case 57:
        case 105:
            //9
            viewportCoords={x:canvasCurrentWH.width/4, y:canvasCurrentWH.height/4, w:canvasCurrentWH.width/4, h:canvasCurrentWH.height/4};
            resize(1);
            break;

        default:
            resize(0);
            break;
        
    }
}

const mouseEvent = function() {
    //Code!
}

