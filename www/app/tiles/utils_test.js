var fs = require('fs');

window = { };
eval(fs.readFileSync('utils.js')+'');

var Utils = this.Utils;

var mat = [ 1,2,3, -4,5,6, 7,-8,9 ];
var inv = Utils.invert3x3Matrix(mat);
var reinvert = Utils.invert3x3Matrix(inv);

for (var i = 0; i < 9; ++i) {
    var diff = Math.abs(mat[i] - reinvert[i]);
    if (diff > .001) {
        console.log('Difference is too large: ' + reinvert[i] + ' should be ' + mat[i]);
    }
}
