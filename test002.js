document.writeln('This is a second test');
var sm = new SailModel002(allnavs.slice(0, 400));
sm.disp();

var stateCount = sm.getStateCount();
var length = sm.getLength();
var reachabilityTable = makeReachabilityTable(sm.connectivity);

var stateCost = sm.getStateCost;
var transitionCost = sm.getTransitionCost;

if (!isStateSpec(sm)) {
    document.writeln('WARNING: The sail model is not a valid state object');
}

for (var i = 0; i < reachabilityTable.length; i++) {
    var arr = reachabilityTable[i];
    for (var j = 0; j < arr.length; j++) {
	document.write(arr[j] + ' ');
    }
    document.writeln('<br />');
}

//assert(isStateSpec(sm), "Not a state spec");

 var X = optimizeStateAssignment(sm, reachabilityTable);

//makeColFromArray(X).disp();
sm.dispLabeledStates(X);

// X.disp();

// document.writeln('Time to solve HMM');

writebr('DONE');
