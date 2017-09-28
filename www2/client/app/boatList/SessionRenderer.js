// This function takes a map of id-to-session,
// and flattens the values into an array, then
// sorts them chronologically.
function sessionMapToArray(m) {
  var dst = [];
  for (var k in m) {
    dst.push(m[k]);
  }
  return _.sortBy(dst, ["startTime"]);
}

// This function applies all the edits
// to an array of rawSessions, and produces 
// a session tree.
function makeSessionTree(rawSessions, edits) {
  return edits.reduce(
    SessionOps.applyEdit,
    SessionOps.buildSessionTree(rawSessions));
}

// This function is needed in case we 
// have two new sessions resulting from splitting 
// a session. Those session will have their _ids removed
function assignSessionId(session) {
  anemoutils.assert(session.boat, "No boat id");
  anemoutils.assert(session.startTime instanceof Date, "Bad start time");
  anemoutils.assert(session.endTime instanceof Date, "Bad end time");
  if (!session._id) {
    session._id = makeCurveId(
      session.boat, 
      session.startTime,
      session.endTime);
  }
  return session;
}

// Takes a function 'addSession', an initial empty collection
// e.g. ( [] or {} ) and a tree. Populates the provided collection
// with the data from the tree, using 'addSession'.
function renderSessions(addSession, initialCollection, tree) {
  return SessionOps.reduceSessionTreeLeaves(
    anemoutils.map(assignSessionId)(addSession), 
    initialCollection, tree);
}

// This function can be passed to .reduce
function addSessionToMap(m, session) {
  m[session._id] = session;
  return m;
}

// A SessionRenderer is responsible for providing a consistent view
// of the sessions with edits applied.
//
// Internally, it uses ValueState objects to cache state
// so that we don't have to recompute it when it is not needed.
// Whenever we call the .get() method on one of the ValueState's,
// we can be certain to get an up-to-date value that is held 
// by that ValueState
function SessionRenderer() {
  // Map of id to session. Used to detect duplicates
  this.idToSession = new anemoutils.ValueState();
  this.idToSession.set({});

  // Array of edits
  this.edits = new anemoutils.ValueState();
  this.edits.set([]);

  // Array of raw sessions
  this.rawSessions = new anemoutils.ValueState(
    sessionMapToArray, [this.idToSession]);

  // The session tree, after all edits were applied
  this.renderedTree = new anemoutils.ValueState(
    makeSessionTree, [this.rawSessions, this.edits]);

  // Rendered sessions, in the form of an array
  this.renderedArray = new anemoutils.ValueState(
    function(tree) {
      return renderSessions(anemoutils.push, [], tree);
    }, [this.renderedTree]);

  // A map from all the rendered sessions.
  this.renderedMap = new anemoutils.ValueState(
    function(tree) {
      return renderSessions(addSessionToMap, {}, tree);
    }, [this.renderedTree]);
}

// This adds a session to the SessionRenderer. Next time any of 
// the ValueState objects are queried, they will return 
// a value that has taken that into account.
SessionRenderer.prototype.addSession = function(session) {
  var updated = true;
  this.idToSession.update(function(m) {
    if (session._id in m) {
      updated = false;
    } else {
      m[session._id] = SessionOps.normalizeSession(session);
    }
    return m;
  });
  return updated;
}

// This adds an edit operation to the SessionRenderer, and
// that operation will be taken into account next time.
SessionRenderer.prototype.addEdit = function(edit) {
  //TODO: Small optimization: If the renderedTree is up-to-date,
  // then we can maybe directly apply this edit to that object.
  // For that, we would need some method .forceUpToDateWithValue,
  // that simply sets the value, and fools the object to believe
  // that that value is up-to-date w.r.t. its dependencies.

  this.edits.update(function(edits) {
    return anemoutils.push(edits, edit);
  });
}
