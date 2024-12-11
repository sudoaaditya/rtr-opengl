//global variable

var canvas = null;
var gl = null; //webgl context
var bFullScreen = false;
var canvas_original_width;
var canvas_original_height;


const WebGLMacros = {

	AMC_ATTRIBUTE_VERTEX:0,
	AMC_ATTRIBUTE_COLOR:1,
	AMC_ATTRIBUTE_NORMAL:2,
	AMC_ATTRIBUTE_TEXTURE0:3
};


var vertexShaderObject;
var fragmentShaderObject;
var shaderProgramObject;

var vao;
var vbo;
var mvpUniform;

var orthographicProjectionMatrix;

var requestAnimationFrame = 
	window.requestAnimationFrame ||
	window.webkitRequestAnimationFrame ||
	window.mozRequestAnimationFrame ||
	window.oRequestAnimationFrame ||
	window.msRequestAnimationFrame ||
	null;


var cancleAnimationFrame = 
	window.cancleAnimationFrame ||
	window.webkitCancleRequestAnimationFrame || window.webkitCancleAnimationFrame ||
	window.mozCancleRequestAnimationFrame || window.mozCancleAnimationFrame ||
	window.oCancleRequestAnimationFrame || window.oCancleAnimationFrame ||
	window.msCancleRequestAnimationFrame || window.msCancleAnimationFrame ||
	null;


//onload function

function main()
{
	//get canvas element
	canvas = document.getElementById("AMC");
	if(!canvas)
		console.log("Obtaining canvas failed!" );
	else	
		console.log("Obtaining canvas succeeded!" );
	
	//print canvas width and height on console
	console.log("Canvas Width:"+canvas.width+" and Canvas height:"+canvas.height);
	
	canvas_original_width = canvas.width;
	canvas_original_height = canvas.height;


	//register keyborad and mouse event handler
	
	window.addEventListener("keydown" , keyDown , false);
	window.addEventListener("click" , mouseDown , false);
	window.addEventListener("resize" , resize , false);

	init();

	resize();
	draw();
}



function toggleFullScreen()
{
	//code
	var fullscreen_element = document.fullscreenElement || 
							 document.webkitFullscreenElement ||
							 document.mozFullScreenElement ||
							 document.msFullscreenElement ||
							 null;
							 
	if(fullscreen_element == null)
	{
		if(canvas.requestFullscreen)
			canvas.requestFullscreen();
		else if(canvas.mozRequestFullScreen)
			canvas.mozRequestFullScreen();
		else if(canvas.webkitRequestFullscreen)
			canvas.webkitRequestFullscreen();
		else if(canvas.msRequestFullscreen)
			canvas.msRequestFullscreen();
		bFullScreen = true;
	}
	else{
		if(document.exitFullscreen)
			document.exitFullscreen();
		else if(document.mozCancelFullScreen)
			document.mozCancelFullScreen();
		else if(document.webkitExitFullscreen)
			document.webkitExitFullscreen();
		else if(document.msExitFullscreen)
			document.msExitFullscreen();
		bFullScreen = false;
	}
}


function init()
{
	gl = canvas.getContext("webgl2");

	if(gl == null)
		console.log("Obtaining  context failed!" );
	else	
		console.log("Obtaining context succeeded!" );

	gl.viewportWidth = canvas.width;
	gl.viewportHeight = canvas.height;

	var vertexShaderSourceObject =
	"#version 300 es" +
	"\n" +
	"in vec4 vPosition;" +
	"uniform mat4 u_mvp_matrix;" +
	"void main(void)" +
	"{" +
	"gl_Position = u_mvp_matrix * vPosition;" +
	"}" ;

	vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShaderObject,vertexShaderSourceObject);
	gl.compileShader(vertexShaderObject);

	if(gl.getShaderParameter(vertexShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error = gl.getShaderInfoLog(vertexShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	var fragmentShaderSourceObject =
	"#version 300 es" +
	"\n" +
	"precision highp float;" +
	"out vec4 FragColor;" +
	"void main(void)" +
	"{" +
	"FragColor = vec4(1.0,1.0,1.0,1.0);" +
	"}" ;

	fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShaderObject,fragmentShaderSourceObject);
	gl.compileShader(fragmentShaderObject);

	if(gl.getShaderParameter(fragmentShaderObject,gl.COMPILE_STATUS)==false)
	{
		var error = gl.getShaderInfoLog(fragmentShaderObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	shaderProgramObject = gl.createProgram();
	gl.attachShader(shaderProgramObject,vertexShaderObject);
	gl.attachShader(shaderProgramObject,fragmentShaderObject);

	gl.bindAttribLocation(shaderProgramObject,WebGLMacros.AMC_ATTRIBUTE_VERTEX,"vPosition");

	gl.linkProgram(shaderProgramObject);

	if(!gl.getProgramParameter(shaderProgramObject,gl.LINK_STATUS))
	{
		var error = gl.getProgramInfoLog(shaderProgramObject);
		if(error.length > 0)
		{
			alert(error);
			uninitialize();
		}
	}

	mvpUniform = gl.getUniformLocation(shaderProgramObject,"u_mvp_matrix");

	var triangleVertices = new Float32Array([
		0.0,50.0,0.0,
		-50.0,-50.0,0.0,
		50.0,-50.0,0.0

		]);


	vao = gl.createVertexArray();
	gl.bindVertexArray(vao);


	vbo = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER,vbo);
	gl.bufferData(gl.ARRAY_BUFFER,triangleVertices,gl.STATIC_DRAW);
	gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_VERTEX,3,gl.FLOAT,false,0,0);
	gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_VERTEX);
	gl.bindBuffer(gl.ARRAY_BUFFER,null);
	gl.bindVertexArray(null);

	gl.clearColor(0.0,0.0,1.0,1.0);

	orthographicProjectionMatrix = mat4.create();

}


function resize()
{
	if(bFullScreen == true)
	{
		canvas.width = window.innerWidth;
		canvas.height = window.innerHeight;
	}
	else
	{
		canvas.width = canvas_original_width;
		canvas.height = canvas_original_height;
	}

	gl.viewport(0,0,canvas.width,canvas.height);


	if(canvas.width <= canvas.height)
	
		mat4.ortho(orthographicProjectionMatrix,
			-100.0,
			 100.0,
			(-100.0 * (canvas.height / canvas.width)),
			(100.0 * (canvas.height / canvas.width)),
			-100.0,
			100.0);
	else
		mat4.ortho(orthographicProjectionMatrix,
			(-100.0 * (canvas.width / canvas.height)),
			(100.0 * (canvas.width / canvas.height)),
			-100.0,
			100.0,
			-100.0,
			100.0);
	
}


function draw()
{
	gl.clear(gl.COLOR_BUFFER_BIT);

	gl.useProgram(shaderProgramObject);

	var modelViewMatrix = mat4.create();
	var modelViewProjectionMatrix = mat4.create();

	mat4.multiply(modelViewProjectionMatrix,orthographicProjectionMatrix,modelViewMatrix);
	gl.uniformMatrix4fv(mvpUniform,false,modelViewProjectionMatrix);

	gl.bindVertexArray(vao);
	gl.drawArrays(gl.TRIANGLES,0,3);

	gl.bindVertexArray(null);
	gl.useProgram(null);

	requestAnimationFrame(draw,canvas);

}

function uninitialize()
{
	if(vao)
	{
		gl.deleteVertexArray(vao);
		vao = null;
	}

	if(vbo)
	{
		gl.deleteBuffer(vbo);
		vbo = null;	
	}

	if(shaderProgramObject)
	{
		if(fragmentShaderObject)
		{
			gl.detachShader(shaderProgramObject,fragmentShaderObject);
			gl.deleteShader(fragmentShaderObject);
			fragmentShaderObject = null;
		}
		if(vertexShaderObject)
		{
			gl.detachShader(shaderProgramObject,vertexShaderObject);
			gl.deleteShader(vertexShaderObject);
			vertexShaderObject = null;
		}

		gl.deleteProgram(shaderProgramObject);
		shaderProgramObject = null;
	}
}

function keyDown(event)
{
	//code
	switch(event.keyCode)
	{
		case 27:
			uninitialize();
			window.close();
			break;
		case 70:
		 toggleFullScreen();
		 break;
	}
}

function mouseDown()
{
	//code
}