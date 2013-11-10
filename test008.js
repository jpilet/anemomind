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

//$('#treecontainer').html(JSON.stringify(tree));
var ts = makeBasicTreeStyle();

var accstring = ts.renderExpandable(tree, allnavs);
$('#treecontainer').html(accstring);


//var accstring = ti.renderExpandable(tree, allnavs);

