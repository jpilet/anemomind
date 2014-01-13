function writebr(mes) {
    document.writeln(mes + '<br />');
}

function repeatChar(c, count) {
    var s = '';
    for (var i = 0; i < count; i++) {
	s += c;
    }
    return s;
}

function makeFWR(width, s, fillchar) {
    if (!isDefined(fillchar)) {
	fillchar = '0';
    }
    if (isNumber(s)) {
	s = '' + s;
    }
    
    assert((s.length <= width), "The string '" + s
	   + "' of length " + s.length + " exceeds the fixed width " + width);
    return repeatChar(fillchar, width - s.length) + s;
}

function MutableString() {
    this.data = '';
}

MutableString.prototype.add = function(s) {
    this.data = this.data + s;
};

MutableString.prototype.get = function() {
    return this.data;
};

function isMutableString(x) {
    return isDefined(x.data);
}


function isNumber(x) {
    return typeof(x) == "number";
}

function isArray(x) {
    return (x instanceof Array);
}


function isFunction(x) {
    return typeof(x) === "function";
}

function isDefined(x) {
    return typeof(x) != "undefined";
}


function assert(condition, message) {
    if (!condition) {
	alert('Assertion failed: ' + message);
	haltThisProgramByTryingToCallThisUndefinedFunction();
    }
}

function zeros(count) {
    var data = new Array(count);
    for (var i = 0; i < count; i++) {
	data[i] = 0;
    }
    return data;
}

function contains(arr, x) {
    return arr.indexOf(x) != -1;
}


function DOUT(s) {
    document.writeln(s + ': ' + eval(s) + '<br/>');
}

function makeParsableString(arr) {
    if (arr.length == 0) {
	return '';
    }
    else {
	var s = '' + arr[0];
	var len = arr.length;

	for (var i = 1; i < len; i++) {
	    s += '-' + arr[i];
	}
    }
}

// function assert(condition, message) {
//     if (!condition) {
//         throw message || "Assertion failed";
//     }
// }
