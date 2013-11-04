var numwidth = 3;



function makeNamedInt(x, pref) {
    if (!isDefined(pref)) {
	pref = 'n';
    }
    var s = '' + x;
    return pref + repeatChar('0', numwidth - s.length) + s;
}

function indsToString(arr) {
    var count = arr.length;
    var s = '';
    for (var i = 0; i < count; i++) {
	s += makeNamedInt(arr[i]);
    }
    return s;
}

function tos(s) {
    return '\"' + s + '\"';
}

function makeIntSymbol(index) {
    var name = makeNamedInt(index);
    var names = tos(name);
    writebr(name + ' = ');
    writebr('   ' + names + ' acc:' + name + ' {acc[1]++; return acc;}');
    writebr('   / ' + names + ' {return [' + index + ', 1];}');
}



function makeAlternatives(inds) {
    assert(inds.length > 0, "makeAlternatives: Empty list not accepted.");
    var s = '(' + makeNamedInt(inds[0]);
    for (var i = 1; i < inds.length; i++) {
	s += ' / ' + makeNamedInt(inds[i]);
    }
    return s + ')';
}

function groupStates(index, inds) {
    var name = makeNamedInt(index);
    writebr(name + ' = acc:' + makeAlternatives(inds) +
	    '+ {acc.unshift(' + index + '); return acc;}');
}

//writebr(indsToString([0, 1, 2, 3, 4]));
function makeIntSymbols(count) {
    for (var i = 0; i < count; i++) {
	makeIntSymbol(i);
    }
}
