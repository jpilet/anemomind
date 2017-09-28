var _ = require('lodash');
var anemoutils = require('../client/app/anemoutils.js');
var SessionOps = require('../client/app/SessionOps.js');
var assert = require('assert');

// From Courdlineone
var rawSessions = require('./raw_sessions.json');

function stripIrrelevant(x) {
  return {
    startTime: x.startTime, 
    endTime: x.endTime
  };
}

function countFun(dst, x) {
  return dst + 1;
};

// A function that takes a total duration and a session, and adds
// the duration of the session to the total duration.
var addDur = anemoutils.map(SessionOps.sessionDurationSeconds)(_.add);

var sessions = rawSessions.map(SessionOps.normalizeSession);

describe('Edit ops', function() {
  console.log("Number of sessions: %d", rawSessions.length);

  it('Delete an entire session', function() {
    
    var tree = SessionOps.buildSessionTree(sessions.map(stripIrrelevant));
    //console.log(JSON.stringify(tree, null, 4));
    assert(tree.startTime == sessions[0].startTime);
    assert(tree.endTime + '' == sessions[sessions.length-1].endTime);

    var leafCount = SessionOps.reduceSessionTreeLeaves(countFun, 0, tree);

    assert(sessions.length == leafCount);

    var totalDuration = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree);

    console.log("Total duration is " + totalDuration/3600 + " hours");

    var index = 3;
    var sessionToDelete = sessions[index];
    var durationToDelete = SessionOps.sessionDurationSeconds(sessionToDelete);

    var tree2 = SessionOps.applyEdit(tree, {
      type: "delete",
      lower: sessionToDelete.startTime,
      upper: sessionToDelete.endTime
    });

    var totalDuration2 = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree2);
    console.log("New duration: " + totalDuration2/3600 + " hours");
    assert(Math.abs((totalDuration - totalDuration2) - durationToDelete) < 1.0e3);
    
    var leafCount2 = SessionOps.reduceSessionTreeLeaves(countFun, 0, tree2);
    assert(leafCount2 + 1 == leafCount);
  });

  it('Delete from an empty list', function() {
    var tree = SessionOps.buildSessionTree([]);
    assert(tree == null);

    // Flatten this tree
    var renderedSessions = SessionOps.reduceSessionTreeLeaves(
      anemoutils.push, [], tree);

    assert(renderedSessions instanceof Array);
    assert(renderedSessions.length == 0);
  });

  it('Delete a session in the middle, making two sessions', function() {
    var tree = SessionOps.buildSessionTree(sessions);

    var totalDur = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree);

    var index = 3;
    var sessionToDelete = sessions[index];

    var marginSeconds = 2*60*60; // Two hours, in seconds
    var tree2 = SessionOps.applyEdit(tree, {
      type: "delete",
      lower: new Date(sessionToDelete.startTime.getTime() + marginSeconds*1000),
      upper: new Date(sessionToDelete.endTime.getTime() - marginSeconds*1000),
    });

    leafCount2 = SessionOps.reduceSessionTreeLeaves(countFun, 0, tree2);
    assert(leafCount2 == sessions.length + 1);
    totalDur2 = SessionOps.reduceSessionTreeLeaves(addDur, 0, tree2);

    var sessions2 = SessionOps.reduceSessionTreeLeaves(anemoutils.push, [], tree2);

    // Make sure the split session does not inherit the _id of the parent
    // We can assign new ids later.
    assert(!sessions2[index]._id);
    assert(!sessions2[index+1]._id);

    // But the other ones should have...
    assert(sessions2[index-1]._id);
    assert(sessions2[index+2]._id);

    var dif0 = (totalDur - totalDur2);
    var dif1 = SessionOps.sessionDurationSeconds(sessionToDelete) 
        - marginSeconds - marginSeconds;
    assert(Math.abs(dif0 - dif1) < 0.1);
  });
});
