

function Matrix(rows, cols, step, data) { // colmajor
    this.rows = rows;
    this.cols = cols;
    this.step = step;
    this.data = data;
}

function isMatrix(x) {
    return isDefined(x.rows) && isDefined(x.cols) && isDefined(x.step) && isDefined(x.data);
}

function makeMatrix(rows, cols) {
    var data = new Array(rows*cols);
    for (var i = 0; i < data.length; i++) {
	data[i] = 0.0;
    }
    
    return new Matrix(rows, cols, rows, data);
}

function makeMatrixFromArray(rows, cols, arr) {
    assert(rows*cols == arr.length);
    return new Matrix(rows, cols, rows, arr);
}

function makeColFromArray(arr) {
    return makeMatrixFromArray(arr.length, 1, arr);
}


Matrix.prototype.colRange = function(j0, j1) {
    var cols = j1 - j0;
    var fromIndex = j0*this.step;
    var toIndex = j1*this.step;
    return new Matrix(this.rows, cols, this.step,
		      this.data.slice(fromIndex, toIndex));
};

Matrix.prototype.sliceCol = function(j) {
    return this.colRange(j, j + 1);
};

Matrix.prototype.rowRange = function(i0, i1) {
    var rows = i1 - i0;
    var fromIndex = i0;
    var toIndex = this.data.length;
    return new Matrix(rows, this.cols, this.step,
		      this.data.slice(fromIndex, toIndex));
};

Matrix.prototype.sliceRow = function(i) {
  return this.rowRange(i, i + 1);  
};

Matrix.prototype.calcIndex = function(i, j) {
    return i + j*this.step;
};

Matrix.prototype.get = function(i, j) {
    return this.data[this.calcIndex(i, j)];
};

Matrix.prototype.validInds = function(i, j) {
    return 0 <= i && i < this.rows && 0 <= j && j < this.cols;
};

Matrix.prototype.illegalAccess = function(i, j) {
	document.writeln('Tried to access element ' +
			 i + ', ' + j + ' in matrix of size ' +
			 this.rows + 'x' + this.cols + '.<br />');
};

Matrix.prototype.getsafe = function(i, j) {
    if (this.validInds(i, j)) {
	return this.data[this.calcIndex(i, j)];
    }
    else {
	document.write('Matrix.getsafe: ');
	this.illegalAccess(i, j);
	return 0;
    }
};

Matrix.prototype.set = function(i, j, x) {
    this.data[this.calcIndex(i, j)] = x;
};

Matrix.prototype.setAll = function(value) {
    for (var i = 0; i < this.rows; i++) {
	for (var j = 0; j < this.cols; j++) {
	    this.set(i, j, value);
	}
    }
};

Matrix.prototype.disp = function() {
    document.write('<table>');
    for (var i = 0; i < this.rows; i++) {
	document.write('<tr>');
	for (var j = 0; j < this.cols; j++) {
	    document.write('<td>' + this.get(i, j) + '</td>');
	}
	document.write('</tr>');
    }
    document.write('</table>');
};

Matrix.prototype.numel = function() {
    return this.rows*this.cols;
};

Matrix.prototype.empty = function() {
  return this.numel() == 0;  
};

Matrix.prototype.getFirstElement = function() {
  assert(!this.empty());
  return this.get(0, 0);
};

Matrix.prototype.getMin = function() {
    var best = this.getFirstElement();
    var bestI = 0;
    var bestJ = 0;
    for (var i = 0; i < this.rows; i++) {
	for (var j = 0; j < this.cols; j++) {
	    var x = this.get(i, j);
	    if (x < best) {
		best = x;
		bestI = i;
		bestJ = j;
	    }
	}
    }
    return [bestI, bestJ, x];
};


Matrix.prototype.getMax = function() {
    var best = this.getFirstElement();
    var bestI = 0;
    var bestJ = 0;
    for (var i = 0; i < this.rows; i++) {
	for (var j = 0; j < this.cols; j++) {
	    var x = this.get(i, j);
	    if (x > best) {
		best = x;
		bestI = i;
		bestJ = j;
	    }
	}
    }
    return [bestI, bestJ, x];
};
