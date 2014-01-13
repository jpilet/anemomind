// THIS FILE SPECIFIES SAILMODEL002.
// To display the states that are part of it, call (new SailModel002).disp();

// Explanation:
// SailModel002 is a state machine with major states and minor states.
// Every state is part of a major state. State 24 corresponds to the device
// being turned off, all other states correspond to the boat sailing. 
// Every state is also associated with a minor state that corresponds 
// to how the boat is sailing. In total there are 25 states and their major
// and minor states are listed in SailModel002.states.


// This object contains values that can be tuned for SailModel002.
// The cost of being in the wrong minor state is 1.
function SailModel002Settings(timePrice, majorStateCost,
			      majorTransitionCost, minorTransitionCost) {
    // This is a small time-dependent cost for transitions between states when the
    // device is turned on. Its purpose is identify long durations when no data is recorded,
    // for which the cost is 0.
    this.timePrice = timePrice;

    // This is the total constant cost for all minor states that are part of a major state
    this.majorStateCost = majorStateCost;

    // This is the cost that is paid to jump between states that belong to
    // different major states.
    this.majorTransitionCost = majorTransitionCost;

    // This is the cost to jump between minor states.
    this.minorTransitionCost = minorTransitionCost;
}

// These are manually assigned values to start with. In future, we
// may want to optimize these values if we have annotated data.
function makeSailModel002StdSettings() {
    return new SailModel002Settings(0.01, 1, 50, 1);
}



// Specification of a sailmodel
function isUpwind(sr) {
    var twa = Math.abs(getTWAMax180(sr));
    return twa < 90;
}

function isDownwind(sr) {
    return !isUpwind(sr);
}

function isBeamReach(sr) {
    var twa = Math.abs(getTWAMax180(sr));
    return 60 < twa && twa < 120;
}

function isStarboardTack(sr) {
    return getTWAMax180(sr) >= 0;
}

function isPortTack(sr) {
    return !isStarboardTack(sr);
}

function getMinorStateLabel(sr) {
    var offset = 0;
    if (isStarboardTack(sr)) {
	if (isBeamReach(sr)) {
	    return 1;
	}
	if (isUpwind(sr)) {
	    return 0
	}
	return 2;
    }
    else {
	if (isBeamReach(sr)) {
	    return 4;
	}
	if (isUpwind(sr)) {
	    return 5
	}
	return 3;
    }
}



var minorStateCount = 6;
var majorStateCount = 5;

function makeMinorStateConnections() {
    var cons = makeMatrix(minorStateCount, minorStateCount);
    cons.setAll(false);
    for (var i = 0; i < minorStateCount; i++) {
	cons.set(i, i, true);
	var next = (i + 1) % minorStateCount;
	cons.set(i, next, true);
	cons.set(next, i, true);
    }
    return cons;
}

function areConnected(a, b, majorcon, minorcon) {
    if (majorcon.getsafe(a[0], b[0])) {
	if (a[1] == -1 || b[1] == -1) {
	    return true;
	}
	return minorcon.getsafe(a[1], b[1]);
    }
    return false;
}

function makeConnectivityMatrix(states, majorcon, minorcon) {
    var stateCount = states.length;
    var cons = makeMatrix(stateCount, stateCount);
    cons.setAll(false);
    for (var i = 0; i < stateCount; i++) {
    	var a = states[i];
    	for (var j = 0; j < stateCount; j++) {
    	    var b = states[j];
    	    cons.set(i, j, areConnected(a, b, majorcon, minorcon));
    	}
    }
    return cons;
}

function makeMajorStateConnections() {
    var cons = makeMatrix(majorStateCount, majorStateCount);
    cons.setAll(false);
    for (var i = 0; i < majorStateCount; i++) {
	cons.set(i, i, true);
    }
    cons.set(0, 1, true);
    cons.set(0, 2, true);
    cons.set(1, 2, true);
    cons.set(2, 1, true);
    cons.set(1, 3, true);
    cons.set(2, 3, true);
    cons.set(3, 4, true);
    cons.set(4, 3, true); 
    cons.set(3, 0, true); // Connect idle state to before race
    //cons.disp();
    return cons;
}


function calcTransitionCosts(states, con, majorcost, minorcost) {
    var stateCount = states.length;
    
    var costs = makeMatrix(stateCount, stateCount);
    for (var i = 0; i < stateCount; i++) {
	var a = states[i];
	for (var j = 0; j < stateCount; j++) {
	    if (con.getsafe(i, j)) {
		var b = states[j];
		var majc = 0;
		var minc = 0;
		if (a[0] != b[0]) {
		    majc = majorcost;
		}
		if (a[1] != b[1] && a[1] != -1 && b[1] != -1) {
		    minc = minorcost;
		}
		costs.set(i, j, majc + minc);
	    }
	}
    }
    return costs;
}

function getMinorStateCountsPerMajorState(states, majorStateCount) {
    var counts = new Array(majorStateCount);
    for (var i = 0; i < majorStateCount; i++) {
	counts[i] = 0;
    }
    for (var j = 0; j < states.length; j++) {
	var s = states[j];
	counts[s[0]] = counts[s[0]] + 1;
    }
    return counts;
}



function SailModel002(allnavs, settings) {
    if (!isDefined(settings)) {
	settings = makeSailModel002StdSettings();
    }
    this.allnavs = allnavs;
    // this.minorTransitionCost = settings.minorTransitionCost;
    // this.majorTransitionCost = settings.majorTransitionCost;
    // this.majorStateCost = settings.majorStateCost;
    this.settings = settings;

    this.states = [[0, 0], [0, 1], [0, 2], [0, 3], [0, 4], [0, 5],
		  [1, 0], [1, 1], [1, 2], [1, 3], [1, 4], [1, 5],
		  [2, 0], [2, 1], [2, 2], [2, 3], [2, 4], [2, 5],
		  [3, 0], [3, 1], [3, 2], [3, 3], [3, 4], [3, 5],
		  [4, -1]];
    this.majorStateLabels = ['Before race',
			    'Upwind leg',
			    'Downwind leg',
			    'Idle',
			    'Switched off'];
    this.minorStateLabels = ['Starboard tack, upwind',
			    'Starboard tack, beam reach',
			    'Starboard tack, downwind',
			    'Port tack, downwind',
			    'Port tack, beam reach',
			    'Port tack, upwind'];
    this.preferredMinorStates = [[1, 4],
				[0, 5],
				[2, 3],
				[],
				[]];
    this.stateCount = this.states.length;

    // Define the constant state costs

    this.minorStateCounts = getMinorStateCountsPerMajorState(this.states,
							    majorStateCount);

    this.constantStateCosts = zeros(this.stateCount);


    for (var I = 0; I < this.stateCount; I++) {
	var state = this.states[I];
	var mstate = state[0];
	var pref = this.preferredMinorStates[mstate];	
	var msc = this.minorStateCounts[mstate];
	var ms = state[1];
	if (!contains(pref, ms)) {
	    var denom = (msc - pref.length);
	    if (denom > 0) {
		this.constantStateCosts[I] =
		    this.settings.majorStateCost/denom;
	    }
	    else {
		document.writeln('pref.length = ' + pref.length);
		document.writeln('minorStateCounts = ' + this.minorStateCounts[I]);
		document.writeln('DIVISION BY ZERO. denom = ' + denom);
	    }
	}
    }

    

    var minorcon = makeMinorStateConnections();
    var majorcon = makeMajorStateConnections();
    this.connectivity = makeConnectivityMatrix(this.states,
    					       majorcon,
    					       minorcon);
    this.costs = calcTransitionCosts(this.states, this.connectivity,
				     this.settings.majorTransitionCost,
				    this.settings.minorTransitionCost);
};

SailModel002.prototype.getStateLabel = function(index) {
    var state = this.states[index];
    var s = this.majorStateLabels[state[0]];
    var mi = state[1];
    if (mi >= 0) {
	s = s + '---' + this.minorStateLabels[mi];
    }
    return s;
};

SailModel002.prototype.getShortStateLabel = function(index) {
    var state = this.states[index];
    var s = this.majorStateLabels[state[0]];
    var mi = state[1];
    if (mi >= 0) {
	return this.minorStateLabels[mi];
    }
    else {
	return 'Switched off';
    }
};

SailModel002.prototype.getStateCount = function() {
  return this.stateCount;  
};

SailModel002.prototype.getLength = function() {
  return this.allnavs.length;
};


SailModel002.prototype.disp = function () {
    for (var i = 0; i < this.stateCount; i++) {
    	//DOUT('this.prototype.getStateLabel(i)');
	//var lab = 'kattskit'; //this.prototype.getStateLabel(i);
	var lab = this.getStateLabel(i);
	document.writeln('State ' + i + ': ' + lab + '   cost: ' + this.constantStateCosts[i] + '<br/>');
    }
};


SailModel002.prototype.getStateCost = function(timeindex, stateindex) {
    var cstCost = this.constantStateCosts[stateindex];
    var dynCost = 0;
    var state = this.states[stateindex];
    var minorState = state[1];
    var estMinorState = getMinorStateLabel(this.allnavs[timeindex]);
    if (minorState != estMinorState) {
	dynCost = 1.0;
    }
    return cstCost + dynCost;
};

SailModel002.prototype.getTransitionCost = function(timeindex,
						    fromState, toState) {
    assert(timeindex < this.getLength()-1, 'Time index too high');
    var cstCost = this.costs.get(fromState, toState);
    var fromSr = this.allnavs[timeindex];
    var toSr = this.allnavs[timeindex + 1];
    var timePrice = this.settings.timePrice;


    var seconds = getSeconds(toSr) - getSeconds(fromSr);

    var timeCost = timePrice*seconds;

    // To time penalty for the device being turned off
    if (fromState == 24 && toState == 24) {
	timeCost = 0.0;
    }

    var cost = cstCost + timeCost;

    assert(!isNaN(cost), 'WARNING: The cost is nan at time '
	   + timeindex + ' from state ' + fromState + ' to state ' + toState);


    return cost;
};

SailModel002.prototype.dispLabeledStates = function(states) {
    var count = states.length;
    var table = makeMatrix(count, 3);
    for (var i = 0; i < count; i++) {
	table.set(i, 0, i+1);
	var state = states[i];
	table.set(i, 1, '[' + state + ']');
	table.set(i, 2, this.getStateLabel(state));	
    }
    table.disp();
};







// repl.print('Rulle');
// repl.print('Rulledmajo');










// REQUIRES treeinfo
function makeTreeInfo002(sm) {
    var stateCount = sm.getStateCount();
    var labels = new Array(41);
    for (var i = 0; i < stateCount; i++) {
	labels[i] = sm.getShortStateLabel(i);
    }
    labels[25] = 'Top';
    labels[26] = 'Sailing';
    labels[27] = 'In race';
    labels[28] = 'Upwind leg';
    labels[29] = 'Starboard tack';
    labels[30] = 'Port tack';
    labels[31] = 'Downwind leg';
    labels[32] = 'Starboard tack';
    labels[33] = 'Port tack';
    labels[34] = 'Not in race';
    labels[35] = 'Idle';
    labels[36] = 'Starboard tack';
    labels[37] = 'Port tack';
    labels[38] = 'Just before race';
    labels[39] = 'Starboard tack';
    labels[40] = 'Port tack';
    return new TreeInfo(stateCount, labels);
}

