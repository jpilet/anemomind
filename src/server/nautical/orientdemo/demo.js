var mat4 = require('gl-mat4');

function degreesToRadian(val) {
	var newVal;
	newVal = val * (Math.PI / 180.0);
	return newVal;
}

function computeMatrix(orientDegrees) {
  var heading = orientDegrees.heading;
  var pitch = orientDegrees.pitch;
  var roll = orientDegrees.roll;

  var mvMatrix = mat4.create();
  mat4.identity(mvMatrix);

// Copied from 3d.js of the anemobox:

// IMPORTANT: This is The Correct Way(tm) to combine
// heading, pitch, and roll from BNO055 to produce a rotation matrix.
	mat4.rotate(mvMatrix, mvMatrix, degreesToRadian(-heading), [0, 1, 0]);
	mat4.rotate(mvMatrix, mvMatrix, degreesToRadian(-pitch), [1, 0, 0]);
	mat4.rotate(mvMatrix, mvMatrix, degreesToRadian(roll), [0, 0, 1]);

  return mvMatrix;
}

function dispOrient(orient) {
  console.log("The orientation ");
  console.log(orient);
  console.log("Maps to " + computeMatrix(orient));
}

dispOrient({
  heading: 0,
  pitch: 0,
  roll: 0
});

dispOrient({
  heading: 90,
  pitch: 0,
  roll: 0
});

dispOrient({
  heading: 0,
  pitch: 90,
  roll: 0
});

dispOrient({
  heading: 45,
  pitch: 20.0,
  roll: -14.3
});


