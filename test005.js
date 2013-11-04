var sm = new SailModel002(allnavs);

var stateCount = sm.getStateCount();
var length = sm.getLength();
var reachabilityTable = makeReachabilityTable(sm.connectivity);

var stateCost = sm.getStateCost;
var transitionCost = sm.getTransitionCost;

var X = optimizeStateAssignment(sm, reachabilityTable);
var tree = parser002.parse(indsToString(X));
writebr(JSON.stringify(tree));

