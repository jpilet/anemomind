var gl;

var heading = 0;
var roll = 0;
var pitch = 0;

function formatAngle(angle) {
  return angle.toFixed(1) + "&deg;";
}

function webGLStart() {
	var canvas = document.getElementById("Sample01-canvas");
	initGL(canvas);
	initShaders();
	initBuffers();

	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.enable(gl.DEPTH_TEST);

	drawScene();

  setInterval(function() {
    $.ajax({
      url: '/api/live', 
      success: function(data) {
        if (data && data.orient) {
          var orient = data.orient;

          heading = orient.heading;
          roll = orient.roll;
          pitch = orient.pitch;
          $('#heading').html(formatAngle(heading));
          $('#pitch').html(formatAngle(pitch));
          $('#roll').html(formatAngle(roll));
          drawScene();
        }
      }
    });
  }, 100);
}

function initGL(canvas) {
	try {
		gl = canvas.getContext("experimental-webgl");
		gl.viewportWidth = canvas.width;
		gl.viewportHeight = canvas.height;
	} catch (e) {
	}
	if (!gl) {
		alert("Could not initialise WebGL");
	}
}

function getShader(gl, id) {
	var shaderScript = document.getElementById(id);
	if (!shaderScript) {
		return null;
	}

	var str = "";
	var k = shaderScript.firstChild;
	while (k) {
		if (k.nodeType == 3) {
			str += k.textContent;
		}
		k = k.nextSibling;
	}

	var shader;
	if (shaderScript.type == "x-shader/x-fragment") {
		shader = gl.createShader(gl.FRAGMENT_SHADER);
	} else if (shaderScript.type == "x-shader/x-vertex") {
		shader = gl.createShader(gl.VERTEX_SHADER);
	} else {
		return null;
	}

	gl.shaderSource(shader, str);
	gl.compileShader(shader);

	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		alert(gl.getShaderInfoLog(shader));
		return null;
	}

	return shader;
}

var shaderProgram;

function initShaders() {
	var fragmentShader = getShader(gl, "shader-fs");
	var vertexShader = getShader(gl, "shader-vs");

	shaderProgram = gl.createProgram();
	gl.attachShader(shaderProgram, vertexShader);
	gl.attachShader(shaderProgram, fragmentShader);
	gl.linkProgram(shaderProgram);

	if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
		alert("Could not initialise shaders");
	}

	gl.useProgram(shaderProgram);

	shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram,
			"aVertexPosition");
	gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

	shaderProgram.vertexColorAttribute = gl.getAttribLocation(shaderProgram,
			"aVertexColor");
	gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);

	shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram,
			"uPMatrix");
	shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram,
			"uMVMatrix");
}

var mvMatrix = mat4.create();
var pMatrix = mat4.create();

function setMatrixUniforms() {
	gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
	gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
}

var triangleVertexPositionBuffer;
var triangleVertexColorBuffer;

var squareVertexPositionBuffer;
var squareVertexColorBuffer;
var cubeVertexIndexBuffer;

var circleVertexPositonBuffer;
var circleVertexColorBuffer;

function initBuffers() {
	//Triangle:
	triangleVertexPositionBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexPositionBuffer);
	var vertices = [
	// Front face
	0.0, 1.0, 0.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0,
	// Right face
	0.0, 1.0, 0.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0,
	// Back face
	0.0, 1.0, 0.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0,
	// Left face
	0.0, 1.0, 0.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0 ];
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
	triangleVertexPositionBuffer.itemSize = 3;
	triangleVertexPositionBuffer.numItems = 12;
	// Triangle Color:
	triangleVertexColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexColorBuffer);	
	var colors = [
	              // Front face
	              1.0, 0.0, 0.0, 1.0,
	              0.0, 1.0, 0.0, 1.0,
	              0.0, 0.0, 1.0, 1.0,
	              // Right face
	              1.0, 0.0, 0.0, 1.0,
	              0.0, 0.0, 1.0, 1.0,
	              0.0, 1.0, 0.0, 1.0,
	              // Back face
	              1.0, 0.0, 0.0, 1.0,
	              0.0, 1.0, 0.0, 1.0,
	              0.0, 0.0, 1.0, 1.0,
	              // Left face
	              1.0, 0.0, 0.0, 1.0,
	              0.0, 0.0, 1.0, 1.0,
	              0.0, 1.0, 0.0, 1.0
	          ];
	
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
	triangleVertexColorBuffer.itemSize = 4;
	triangleVertexColorBuffer.numItems = 12;

	//Square:
	squareVertexPositionBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexPositionBuffer);
	vertices = [
	            // Front face
	            -1.0, -1.0,  1.0,
	             1.0, -1.0,  1.0,
	             1.0,  1.0,  1.0,
	            -1.0,  1.0,  1.0,

	            // Back face
	            -1.0, -1.0, -1.0,
	            -1.0,  1.0, -1.0,
	             1.0,  1.0, -1.0,
	             1.0, -1.0, -1.0,

	            // Top face
	            -1.0,  1.0, -1.0,
	            -1.0,  1.0,  1.0,
	             1.0,  1.0,  1.0,
	             1.0,  1.0, -1.0,

	            // Bottom face
	            -1.0, -1.0, -1.0,
	             1.0, -1.0, -1.0,
	             1.0, -1.0,  1.0,
	            -1.0, -1.0,  1.0,

	            // Right face
	             1.0, -1.0, -1.0,
	             1.0,  1.0, -1.0,
	             1.0,  1.0,  1.0,
	             1.0, -1.0,  1.0,

	            // Left face
	            -1.0, -1.0, -1.0,
	            -1.0, -1.0,  1.0,
	            -1.0,  1.0,  1.0,
	            -1.0,  1.0, -1.0,
	          ];
	
	
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
	squareVertexPositionBuffer.itemSize = 3;
	squareVertexPositionBuffer.numItems = 24;
	//Square Color:
	squareVertexColorBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexColorBuffer);
	colors = [ [ 1.0, 0.0, 0.0, 1.0 ], // Front face
	[ 1.0, 1.0, 0.0, 1.0 ], // Back face
	[ 0.0, 1.0, 0.0, 1.0 ], // Top face
	[ 1.0, 0.5, 0.5, 1.0 ], // Bottom face
	[ 1.0, 0.0, 1.0, 1.0 ], // Right face
	[ 0.0, 0.0, 1.0, 1.0 ], // Left face
	];
	var unpackedColors = [];
	for ( var i in colors) {
		var color = colors[i];
		for ( var j = 0; j < 4; j++) {
			unpackedColors = unpackedColors.concat(color);
		}
	}
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(unpackedColors), gl.STATIC_DRAW);
	squareVertexColorBuffer.itemSize = 4;
	squareVertexColorBuffer.numItems = 24;
	

	    cubeVertexIndexBuffer = gl.createBuffer();
	    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
	    var cubeVertexIndices = [
	      0, 1, 2,      0, 2, 3,    // Front face
	      4, 5, 6,      4, 6, 7,    // Back face
	      8, 9, 10,     8, 10, 11,  // Top face
	      12, 13, 14,   12, 14, 15, // Bottom face
	      16, 17, 18,   16, 18, 19, // Right face
	      20, 21, 22,   20, 22, 23  // Left face
	    ]
	    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(cubeVertexIndices), gl.STATIC_DRAW);
	    cubeVertexIndexBuffer.itemSize = 1;
	    cubeVertexIndexBuffer.numItems = 36;

}

function degreesToRadian(val) {
	var newVal;
	newVal = val * (Math.PI / 180.0);
	return newVal;
}

function drawScene() {
	gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	mat4.perspective(45, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0,
			pMatrix);

	mat4.identity(mvMatrix);

	mat4.translate(mvMatrix, [ -1.0, 1.0, -6.0 ]);

        // IMPORTANT: This is The Correct Way(tm) to combine
        // heading, pitch, and roll from BNO055 to produce a rotation matrix.
	mat4.rotate(mvMatrix, degreesToRadian(-heading), [0, 1, 0]);
	mat4.rotate(mvMatrix, degreesToRadian(-pitch), [1, 0, 0]);
	mat4.rotate(mvMatrix, degreesToRadian(roll), [0, 0, 1]);

	//Square:
	//mat4.translate(mvMatrix, [ 2.8, -2.8, 0.0 ]);
	gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexPositionBuffer);
	gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute,
			squareVertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);
	gl.bindBuffer(gl.ARRAY_BUFFER, squareVertexColorBuffer);	
	gl.vertexAttribPointer(shaderProgram.vertexColorAttribute,
			squareVertexColorBuffer.itemSize, gl.FLOAT, false, 0, 0);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cubeVertexIndexBuffer);
	setMatrixUniforms();
	gl.drawElements(gl.TRIANGLES, cubeVertexIndexBuffer.numItems, gl.UNSIGNED_SHORT, 0);
}
