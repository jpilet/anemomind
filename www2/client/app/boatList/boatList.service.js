'use strict';

function sessionMapToArray(m) {
  var dst = [];
  for (var k in m) {
    dst.push(m[k]);
  }
  dst.sort(anemoutils.compareByKey("startTime")); // By what
  return dst;
}

function makeSessionTree(rawSessions, edits) {
  return edits.reduce(
    SessionOps.applyEdit,
    SessionOps.buildSessionTree(rawSessions));
}

// This function is needed in case we 
// have two new sessions resulting from splitting 
// a session.
function assignSessionId(session) {
  anemoutils.assert(session.boat);
  anemoutils.assert(session.startTime instanceof Date);
  anemoutils.assert(session.endTime instanceof Date);
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

function addSessionToMap(m, session) {
  m[session._id] = session;
  return m;
}

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

SessionRenderer.prototype.addSession = function(session) {
  var updated = true;
  this.idToSession.update(function(m) {
    if (session._id in m) {
      updated = false;
    } else {
      m[session._id] = session;
    }
    return m;
  });
  return updated;
}

SessionRenderer.prototype.addEdit = function(edit) {
  this.edits.update(function(edits) {
    return anemoutils.push(edits, edit);
  });
}

angular.module('www2App')
  .service('boatList', function (Auth, $http, $q,socket, $rootScope,$log) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};
    var perBoatData = {};

    // Either 'anonymous', or a username, or undefined.
    // Used to cache requests
    var loadedFor;

    // if defined, contain the currently loading promise for boats()
    var loading;

    function clear() {
      boats = [ ];
      boatDict = { };
      curves = { }; // Map from session id to some data.

      // Stores the edited set of sessions, as they should be
      // displayed on the client.
      sessionsForBoats = {}; 

      // Stores all the extra hidden state regarding the boats,
      // such as per-boat edits, raw server sessions, etc.
      perBoatData = {}; // Map from boatId to data related to that boat

      loadedFor = undefined;
      loading = undefined;
      console.log('Forgetting boat data');
    }

    // Forget anything loaded (called on logout)
    $rootScope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      if ((oldVal && !newVal) || (!oldVal && newVal)) {
        // logout or login, forget what we have loaded.
        clear();
      }
    });

    // update local dict and array of boats
    function updateBoatRepo(boat) {
      boatDict[boat._id] = boat;
      for (var i in boats) {
        if (boats[i]._id == boat._id) {
          boats[i] = boat;
          return;
         }
      }
      boats.push(boat);
    }

    function firstEntryMatchingField(array, field, value) {
      for (var i = 0; i < array.length; ++i) {
        if (array[i][field] === value) {
          return array[i];
        }
      }
      return null;
    }

    function updateSessionRepo(newSessions) {

      // Insert all the sessions, that may
      // belong to different boats
      for (var i in newSessions) {
        var newSession = newSessions[i];
        var path = [newSession.boat, "sessions"];
        anemoutils.updateIn(
          perBoatData, path,
          function(renderer0) {
            var renderer = renderer0 || new SessionRenderer();
            if (renderer.addSession(newSession)) {
              curves[newSession._id] = newSession;
            }
            return renderer;
          });
      }
      
      // Loop over the boats and produce the rendered sessions
      for (var boatId in perBoatData) {
        var srcPath = [boatId, "sessions"];
        var dstPath = [boatId];
        var renderer = anemoutils.getIn(perBoatData, srcPath);
        anemoutils.setIn(
          sessionsForBoats, dstPath, 
          renderer.renderedArray.get());
      }
    }

    // return the default boat to display at home
    function getDefaultBoat() {
      if(!boats.length){
        return undefined;
      }

      var bestBoat;
      var mostRecentSession;
      for (var boat in sessionsForBoats) {
        var sessions = sessionsForBoats[boat];
        for (var j in sessions) {
          var s = sessions[j];
          var time = new Date(s.endTime);
          if (!bestBoat || mostRecentSession < time) {
            mostRecentSession = time;
            bestBoat = boat;
          }
        }
      }
      return (bestBoat ? boatDict[bestBoat] : boats[boats.length - 1]);
    }

    function update() {
      // promise for boats methods;
      var deferred = $q.defer();
      var promise=deferred.promise;
      loading = promise;

      // Specifically do not ask for public boats.
      // We could have a user option to show public boats with:
      // $http.get('/api/boats?public=1')
      $http.get('/api/boats')
        .then(function(payload) {

          for (var i in payload.data) {
            updateBoatRepo(payload.data[i]);
          }
          socket.syncUpdates('boat', boats);
          $rootScope.$broadcast('boatList:updated', boats);

          // chain session loading
          return $http.get('/api/session');
        })
        .then(function(payload) {
          updateSessionRepo(payload.data);

          // time to resolve promise
          $rootScope.$broadcast('boatList:sessionsUpdated', sessionsForBoats);

          loadedFor = userOrAnomymous();
          loading = undefined;
          deferred.resolve(boats);
        });

      return promise;
    }

    

    function fetchBoat(boatid) {
      // promise for boats methods;
      var deferred = $q.defer();
      var promise=deferred.promise;

      $http.get('/api/boats/' + boatid)
        .then(function(payload) {
          var boat = payload.data;
          updateBoatRepo(boat);

          $rootScope.$broadcast('boatList:updated', boats);

          // chain session loading
          return $http.get('/api/session/boat/' + boatid);
        })
        .then(function(payload) {
          updateSessionRepo(payload.data);
          $rootScope.$broadcast('boatList:sessionsUpdated', sessionsForBoats);

          loading = undefined;
          deferred.resolve(boatDict[boatid]);
        });

      return promise;
    }

    function userOrAnomymous() {
      return (Auth.isLoggedIn() ? Auth.getCurrentUser() : 'anonymous');
    }

    function cachedBoats() {
      if (loading) {
        return loading;
      }
      if (loadedFor == userOrAnomymous()) {
        var d = $q.defer();
        d.resolve(boats);
        return d.promise;
      }
      return update();
    }


    function save(id,boat) {
      var promise=$http.put('/api/boats/' + id, boat);
      promise.success(updateBoatRepo);
      return promise;

    }

    function addMember(id,invitation) {
      var promise=$http.put('/api/boats/' + id + '/invite', invitation);
      promise.success(updateBoatRepo);
      return promise;
    }


    // TODO
    function locationForCurve(curveId) {
      if (!(curveId in curves)) {
        return undefined;
      }
      var c = curves[curveId];
      return c.location;
    }

    function deleteSession(boatId, sessionId) {
      var session = firstEntryMatchingField(
        sessionsForBoats[boatId], '_id', sessionId);
      anemoutils.assert(session, "No session found");
      var op = {
        type: "delete",
        boat: boatId,
        lower: session.startTime,
        upper: session.endTime,
        creationTime: new Date()
      };
      anemoutils.updateIn(perBoatData, [boatId, "edits"], function(x) {
        var dst = x || [];
        dst.push(op);
        return dst;
      });

      var edits = anemoutils.getIn(perBoatData, [boatId, "edits"]);
      console.log("----------- All edits");
      for (var i in edits) {
        console.log("Edit: ");
        console.log(edits[i]);
      }

      alert('deleteSession');
    }

    //
    // service result
    return {
      boat: function(id) {
              if (id in boatDict && boatDict[id]) {
                return $q(function(resolve){ resolve(boatDict[id]); });
              } else {
                return fetchBoat(id);
              }
            },
      save: save,
      addMember:addMember,
      boats: cachedBoats,
      sessions: function() { return $.extend({}, sessionsForBoats); },
      sessionsForBoat: function(boatId) { return sessionsForBoats[boatId]; },

      // This function is no longer used.
      // getCurveData: function(curveId) { return curves[curveId]; },

      getDefaultBoat: getDefaultBoat,
      locationForCurve: locationForCurve,
      update: cachedBoats,
      deleteSession: deleteSession
    };
  });
