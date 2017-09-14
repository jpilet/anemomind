'use strict';

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

      // TODO: Consider moving this into perBoatData.
      sessionsForBoats = {}; // Map from boatId to array of sessions

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
      for (var i in newSessions) {
        var newSession = newSessions[i];
        if (!(newSession.boat in sessionsForBoats)) {
          sessionsForBoats[newSession.boat] = [ ];
        }
        var sessionsForBoat = sessionsForBoats[newSession.boat];

        // make sure we do not duplicate sessions
        var session = firstEntryMatchingField(
          sessionsForBoat, '_id', newSession._id);
        if (!session) {
          sessionsForBoat.push(newSession);
          curves[newSession._id] = newSession;
        }
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
      assert(session, "No session found");
      var op = {
        type: "delete",
        boatId: boatId,
        lower: session.startTime,
        upper: session.endTime
      };
      accessKey(perBoatData, boatId, []).push(op);
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
      getCurveData: function(curveId) { return curves[curveId]; },
      getDefaultBoat: getDefaultBoat,
      locationForCurve: locationForCurve,
      update: cachedBoats,
      deleteSession: deleteSession
    };
  });
