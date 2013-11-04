function isStateSpec(obj) {
    return isFunction(obj.getStateCount) && isFunction(obj.getLength)
        isFunction(obj.getTransitionCost) && isFunction(obj.getStateCost)
	&& isDefined(obj.connectivity);
}



// State
function makeReachabilityTable(con) {
    var count = con.rows;
    if (count != con.cols) {
	document.writeln('WARNING: Connectivity table not square.');
    }

    var table = new Array(count);
    for (var j = 0; j < count; j++) {
	var predecessors = new Array(0);
	for (var i = 0; i < count; i++) {
	    if (con.get(i, j)) {
		predecessors.push(i);
	    }
	    table[j] = predecessors;
	}
    }
    
    return table;
}

function initializeStateCosts(stateSpec) {
    var costs = makeMatrix(stateSpec.getLength(), stateSpec.getStateCount());
    costs.setAll(0.0);
    for (var i = 0; i < stateSpec.getStateCount(); i++) {
	costs.set(0, i, stateSpec.getStateCost(0, i));
    }
    return costs;
}

function initializeStateBackptrs(stateSpec) {
    var backptrs = makeMatrix(stateSpec.getLength(),
			      stateSpec.getStateCount());
    backptrs.setAll(-1);
    return backptrs;
}

function calcBestPredecessor(time, stateSpec, costs, predecessors, dst) {
    var count = predecessors.length;
    var bestIndex = -1;
    var bestCost = 1.0e30;
    assert(count > 0, 'No predecessors provided');
    for (var i = 0; i < count; i++) {
	var src = predecessors[i];
	var acccost = costs.get(time, src);
	var trcost = stateSpec.getTransitionCost(time, src, dst);
	var cost = acccost + trcost;
	if (cost < bestCost) {
	    bestIndex = src;
	    bestCost = cost;
	}
    }
    return [bestCost, bestIndex];
}

function fillStateCostsAndPtrsAtTime(time, stateSpec, backptrs, costs,
				     reachTable) {
    var stateCount = stateSpec.getStateCount();
    for (var j = 0; j < stateCount; j++) {
	var costAndPtr = calcBestPredecessor(time-1, stateSpec, costs,
					     reachTable[j], j);
	var predcost = costAndPtr[0];
	var ptr = costAndPtr[1];
	backptrs.set(time, j, ptr);
	costs.set(time, j, predcost + stateSpec.getStateCost(time, j));
    }
}

function findLowestCostTermination(costs) {
    assert(isMatrix(costs));
    var lastRow = costs.sliceRow(costs.rows-1);
    var minLoc = costs.getMin();
    return minLoc[1];
}

function fillStateCostsAndPtrs(stateSpec, backptrs, costs, reachTable) {
    var length = stateSpec.getLength();

    
    for (var i = 1; i < length; i++) {
	if (i % 1000 == 0) {
	    console.log('Iteration ' + i);
	}
	fillStateCostsAndPtrsAtTime(i, stateSpec, backptrs, costs,
				    reachTable);
    }
}

function unwindStates(stateSpec, backptrs, costs) {
    var length = stateSpec.getLength();
    var states = new Array(length);
    states[length-1] = findLowestCostTermination(costs);
    for (var i = length-2; i >= 0; i--) {
	var next = i + 1;
	states[i] = backptrs.get(next, states[next]);
    }
    for (i = 0; i < length; i++) {
	
    }
    return states;
}


function optimizeStateAssignment(stateSpec, reachabilityTable) {

    assert(isDefined(stateSpec), "StateSpec undefined");
    assert(isStateSpec(stateSpec),
	   "optimizeStateAssignment: Not a StateSpec");
    assert(isDefined(reachabilityTable), "Reachability table undefined");
    var stateCount = stateSpec.getStateCount();
    var length = stateSpec.getLength();
    
    var backptrs = initializeStateBackptrs(stateSpec);
    var costs = initializeStateCosts(stateSpec);
    
    fillStateCostsAndPtrs(stateSpec, backptrs, costs, reachabilityTable);
    return unwindStates(stateSpec, backptrs, costs);
}

