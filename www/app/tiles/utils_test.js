var fs = require('fs');

window = { };
document = false;
eval(fs.readFileSync('utils.js')+'');

var Utils = this.Utils;

function checkNear(value, target, tolerance) {
  var diff = Math.abs(value - target);
  if (diff > .001) {
      console.log('Difference is too large: ' + value + ' should be ' + target);
  }
}

var mat = [ 1,2,3, -4,5,6, 7,-8,9 ];
var inv = Utils.invert3x3Matrix(mat);
var reinvert = Utils.invert3x3Matrix(inv);

for (var i = 0; i < 9; ++i) {
  checkNear(mat[i], reinvert[i], .001);
}

var identity = Utils.multiply3x3Matrices(mat, inv);

for (var i = 0; i < 3; ++i) {
  for (var j = 0; j < 3; ++j) {
    checkNear(identity[i*3 + j], (i == j ? 1: 0), .0001);
  }
}

