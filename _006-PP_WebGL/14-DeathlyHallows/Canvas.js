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

var vaoTriangle, vboTrianglePos;
var vaoCircle, vboCirclePos;
var vaoLine, vboLinePos;

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

    var fx1, fx2, fx3, fy1, fy2, fy3;
    var fArea, fRad, fPer;
    var fdAB, fdBC, fdAC;
    var fxCord, fyCord;

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
            "   gl_PointSize = 1.0;" +
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
            "   FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
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

    //Related TO Triangle
    fx1 = 0.0;
    fy1 = 1.0;
    fx2 = -1.0;
    fy2 = -1.0;
    fx3 = 1.0;
    fy3 = -1.0;

    var triangleVertices = new Float32Array([
		fx1, fy1, 0.0,
		fx2, fy2, 0.0,
        fx3, fy3, 0.0,
        fx1, fy1, 0.0
        ]);
        

    vaoTriangle = gl.createVertexArray();
    gl.bindVertexArray(vaoTriangle);
    vboTrianglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboTrianglePos);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //InCircle cha Rada!.
        //diatances of each side of triangle by distance formula
    fdAB = Math.sqrt(((fx2 - fx1)*(fx2 - fx1)) + ((fy2 - fy1)*(fy2 - fy1)));
    fdBC = Math.sqrt(((fx3 - fx2)*(fx3 - fx2)) + ((fy3 - fy2)*(fy3 - fy2)));
    fdAC = Math.sqrt(((fx3 - fx1)*(fx3 - fx1)) + ((fy3 - fy1)*(fy3 - fy1)));
            
    //perimeter of triangle >> A+B+C and we need half of it for area 

    fPer = ((fdAB + fdAB + fdBC) / 2);

    //are of T = sqrt(P(P-A)(P-B)(P-C))
    fArea = Math.sqrt(fPer*(fPer - fdAB)*(fPer - fdBC)*(fPer - fdAC));

    //Radius of inCircle = AreaOf T/Perimete Of T
    fRad = (fArea / fPer);

    fxCord = (((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
    fyCord = (((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

    console.log(fxCord, fyCord);

    var circleVert = [];
    var i, j;
    var circleSteps = 0.0;
    for(i = 0; i < 6290; i++) {
        for(j = 0; j < 2; j++) {
            if(j==0)
                circleVert[ (i*2) + j] =  fxCord + Math.cos(circleSteps)*fRad;
            else
                circleVert[ (i*2) + j] =  fyCord + Math.sin(circleSteps)*fRad;
        }
        circleSteps += 0.01;
    }
    vaoCircle = gl.createVertexArray();
    gl.bindVertexArray(vaoCircle);
    vboCirclePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboCirclePos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleVert), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //For Line!
    var lineVer = new Float32Array([ 0.0, 1.0, 0.0, 0.0, -1.0, 0.0]);
    vaoLine = gl.createVertexArray();
    gl.bindVertexArray(vaoLine);
    vboLinePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboLinePos);
    gl.bufferData(gl.ARRAY_BUFFER, lineVer, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);


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
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    gl.bindVertexArray(vaoTriangle);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 1, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoCircle);
    gl.drawArrays(gl.POINTS, 0, 6280);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoLine);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.bindVertexArray(null)

    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
}


const uninitialize = function() {
    //Code!.

    if(vboTrianglePos){
        gl.deleteBuffer(vboTrianglePos);
        vboTrianglePos = null;
    }
    if(vaoTriangle) {
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
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
    }
}

const mouseEvent = function() {
    //Code!
}

