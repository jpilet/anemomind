var sm = new SailModel002(allnavs.slice(0, 300));

var stateCount = sm.getStateCount();
var length = sm.getLength();
var reachabilityTable = makeReachabilityTable(sm.connectivity);

var stateCost = sm.getStateCost;
var transitionCost = sm.getTransitionCost;

var X = optimizeStateAssignment(sm, reachabilityTable);
//makeColFromArray(X).disp();
var tree = parser002.parse(indsToString(X));
var ti = makeTreeInfo002(sm);
//sm.disp();
//sm.connectivity.disp();

var accstring = ti.render(tree, 2, allnavs);
assert(accstring.length > 0, "Accstring should not be empty");
$('#treecontainer').html(accstring);
//writebr('DONE.');
