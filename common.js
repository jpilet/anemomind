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
