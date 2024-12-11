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

var vaoTriangle, vboTrianglePos, vboTriangleCol;
var vaoCircle, vboCirclePos, vboCircleCol;

var vaoAxes, vboAxesPos, vboAxesCol;
var vaoHorUp, vboHorUpPos, vaoHorDwn, vboHorDwnPos;
var vaoRt, vboRtPos, vaoLft, vboLftPos;
var vaoRect, vboRectPos, vboRectCol;
var vaoOutCir, vboOutCirPos, vboOutCirCol;

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
            "in vec4 vColor;" +
            "out vec4 out_vColor;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void) {" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
            "   gl_PointSize = 1.0;" +
            "   out_vColor = vColor;" +
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
            "in vec4 out_vColor;" +
            "void main(void) {" +
            "   FragColor = out_vColor;" +
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
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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

    //VAOs & VBOs Posting
    var axesVert = new Float32Array([5.0, 0.0, 0.0, -5.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, -3.0, 0.0]);
    var axesCol = new Float32Array([0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0]);
        var horLinesVert = [];
        var fSteps = 0.1;
        for(var i = 0; i< 20; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVert[(i*4) + j] = 5.0;
                }
                else if(j == 2) {
                    horLinesVert[(i * 4) + j] = -5.0;
                }
                else {
                    horLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.1;
        }
        var horLinesVertDwn = [];
        fSteps = 0.1;
        for(var i = 0; i< 20; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVertDwn[(i*4) + j] = 5.0;
                }
                else if(j == 2) {
                    horLinesVertDwn[(i * 4) + j] = -5.0;
                }
                else {
                    horLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.1;
        }
        var vertLinesVert = [80];
        fSteps = 0.1;
        for(var i = 0; i< 40; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVert[(i*4) + j] = 3.0;
                }
                else if(j == 3) {
                    vertLinesVert[(i * 4) + j] = -3.0;
                }
                else {
                    vertLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.1;
        }
        var vertLinesVertDwn = [80];
        fSteps = 0.1;
        for(var i = 0; i< 40; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVertDwn[(i*4) + j] = 3.0;
                }
                else if(j == 3) {
                    vertLinesVertDwn[(i * 4) + j] = -3.0;
                }
                else {
                    vertLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.1;
        }
            console.log(horLinesVert);
            console.log(horLinesVertDwn);
            console.log(vertLinesVert);
            console.log(vertLinesVertDwn);
            
        //Axes
        vaoAxes = gl.createVertexArray();
        gl.bindVertexArray(vaoAxes);
        vboAxesPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboAxesPos);
        gl.bufferData(gl.ARRAY_BUFFER, axesVert, gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        vboAxesCol = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboAxesCol);
        gl.bufferData(gl.ARRAY_BUFFER, axesCol, gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.bindVertexArray(null);
        //Hor UP
        vaoHorUp = gl.createVertexArray();
        gl.bindVertexArray(vaoHorUp);
        vboHorUpPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboHorUpPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(horLinesVert), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
        gl.bindVertexArray(null);
        //Hor Down
        vaoHorDwn = gl.createVertexArray();
        gl.bindVertexArray(vaoHorDwn);
        vboHorDwnPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboHorDwnPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(horLinesVertDwn), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
        gl.bindVertexArray(null);
        //Left 
        vaoLft = gl.createVertexArray();
        gl.bindVertexArray(vaoLft);
        vboLftPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboLftPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertLinesVert), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
        gl.bindVertexArray(null);
        //Right
        vaoRt = gl.createVertexArray();
        gl.bindVertexArray(vaoRt);
        vboRtPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboRtPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertLinesVertDwn), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
        gl.bindVertexArray(null);

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

    var triangleCol = new Float32Array([1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0])
        

    vaoTriangle = gl.createVertexArray();
    gl.bindVertexArray(vaoTriangle);
    vboTrianglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboTrianglePos);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboTriangleCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboTriangleCol);
    gl.bufferData(gl.ARRAY_BUFFER, triangleCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
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
    var circleCol = [];
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
        circleCol[(i*2) + 0] = 1.0;
        circleCol[(i*2) + 1] = 1.0;
        circleCol[(i*2) + 2] = 0.0;
    }
    vaoCircle = gl.createVertexArray();
    gl.bindVertexArray(vaoCircle);
    vboCirclePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboCirclePos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleVert), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboCircleCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboCircleCol);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleCol), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    var rectVert = new Float32Array([fx3, fy1, 0.0, fx2, fy1, 0.0, fx2, fy2, 0.0, fx3, fy3, 0.0, fx3, fy1, 0.0]);
    var rectCol = new Float32Array([1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0]);
    vaoRect = gl.createVertexArray();
    gl.bindVertexArray(vaoRect);
    vboRectPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectPos);
    gl.bufferData(gl.ARRAY_BUFFER, rectVert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboRectCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectCol);
    gl.bufferData(gl.ARRAY_BUFFER, rectCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    var outCircVert = [];
    var outCircCol = [];
    var i, j;
    circleSteps = 0.0;
    for(i = 0; i< 6280; i++) {
        for(j = 0; j < 2; j++) {
            if(j == 0)
                outCircVert[ (i*2) + j] = 1.42*Math.cos(circleSteps);
            else
                outCircVert[ (i*2) + j] = 1.42*Math.sin(circleSteps);
        }
        circleSteps += 0.001;
        outCircCol[(i*2) + 0] = 1.0;
        outCircCol[(i*2) + 1] = 1.0;
        outCircCol[(i*2) + 2] = 0.0;
    }
    vaoOutCir = gl.createVertexArray();
    gl.bindVertexArray(vaoOutCir);
    vboOutCirPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboOutCirPos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(outCircVert), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboOutCirCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboOutCirCol);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(outCircCol), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
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

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    gl.bindVertexArray(vaoAxes);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoHorUp);
    for(i = 0; i < 40; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoHorDwn);
    for(i = 0; i < 40; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoLft);
    for(i = 0; i < 80; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoRt);
    for(i = 0; i < 80; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoTriangle);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 1, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoCircle);
    gl.drawArrays(gl.POINTS, 0, 6280);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoRect);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 1, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 3, 2);
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoOutCir);
    gl.drawArrays(gl.POINTS, 0, 6280);
    gl.bindVertexArray(null);
    
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
    if(vboCirclePos){
        gl.deleteBuffer(vboCirclePos);
        vboCirclePos = null;
    }
    if(vaoCircle) {
        gl.deleteVertexArray(vaoCircle);
        vaoCircle = null;
    }
    if(vboHorUpPos){
        gl.deleteBuffer(vboHorUpPos);
        vboHorUpPos = null;
    }
    if(vboHorDwnPos){
        gl.deleteBuffer(vboHorDwnPos);
        vboHorDwnPos = null;
    }
    if(vboLftPos){
        gl.deleteBuffer(vboLftPos);
        vboLftPos = null;
    }
    if(vboRtPos){
        gl.deleteBuffer(vboRtPos);
        vboRtPos = null;
    }
    if(vboAxesPos){
        gl.deleteBuffer(vboAxesPos);
        vboAxesPos = null;
    }
    if(vaoAxes) {
        gl.deleteVertexArray(vaoAxes);
        vaoAxes = null;
    }
    if(vaoHorUp) {
        gl.deleteVertexArray(vaoHorUp);
        vaoHorUp = null;
    }
    if(vaoHorDwn) {
        gl.deleteVertexArray(vaoHorDwn);
        vaoHorDwn = null;
    }
    if(vaoLft) {
        gl.deleteVertexArray(vaoLft);
        vaoLft = null;
    }
    if(vaoRt) {
        gl.deleteVertexArray(vaoRt);
        vaoRt = null;
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

