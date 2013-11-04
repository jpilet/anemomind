var sm = new SailModel002(allnavs.slice(0, 400));

var stateCount = sm.getStateCount();
var length = sm.getLength();
var reachabilityTable = makeReachabilityTable(sm.connectivity);

var stateCost = sm.getStateCost;
var transitionCost = sm.getTransitionCost;

 var X = optimizeStateAssignment(sm, reachabilityTable);

sm.dispLabeledStates(X);

writebr('DONE');
