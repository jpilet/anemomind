function isArray(what) {
    return Object.prototype.toString.call(what) === '[object Array]';
}

function eqArray(a, b) {
    if (a.length == b.length) {
	for (var i = 0; i < a.length; i++) {
	    if (!eq(a[i], b[i])) {
		return false;
	    }
	}
	return true;
    }
    return false;
}

function eqObjectSub(a, b) {
    for (var key in a) {
	if (!eq(a[key], b[key])) {
	    return false;
	}
    }
    return true;
}

function eqObject(a, b) {
    return eqObjectSub(a, b) && eqObjectSub(b, a);
}


function eq(a, b) {
    if (isArray(a)) {
	if (isArray(b)) {
	    return eqArray(a, b);
	}
    } else if (typeof a == 'object') {
	if (typeof b == 'object') {
	    return eqObject(a, b);
	}	
    }
    
    return a == b;
}

function eqv(a, b) {
    var result = eq(a, b);
    if (!result) {
	console.log('Not eq(a, b):');
	console.log(' a = %j', a);
	console.log(' b = %j', b);
    }
    return result;
}

module.exports.eq = eq;
module.exports.eqv = eqv;
