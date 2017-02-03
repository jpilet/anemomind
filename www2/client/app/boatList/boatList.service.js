'use strict';

angular.module('www2App')
  .service('boatList', function (Auth, $http, $q,socket, $rootScope,$log) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};

    // Either 'anonymous', or a username, or undefined.
    // Used to cache requests
    var loadedFor;

    // if defined, contain the currently loading promise for boats()
    var loading;


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
            var boat = payload.data[i];
            boatDict[boat._id] = boat;
            boats.push(boat);
          }
          socket.syncUpdates('boat', boats);
          $rootScope.$broadcast('boatList:updated', boats);

          // chain session loading
          return $http.get('/api/session');
        })
        .then(function(payload) {
          for (var i in payload.data) {
            if (payload.data[i].boat in sessionsForBoats) {
              sessionsForBoats[payload.data[i].boat].push(payload.data[i]);
            } else {
              sessionsForBoats[payload.data[i].boat] = [ payload.data[i] ];
            }
            curves[payload.data[i]._id] = payload.data[i];
          }
          //
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
          if (boat && boat._id && boat.name) {
            boatDict[boat._id] = boat;
          }
          boats.push(boat);

          $rootScope.$broadcast('boatList:updated', boats);

          // chain session loading
          return $http.get('/api/session/boat/' + boatid);
        })
        .then(function(payload) {
          for (var i in payload.data) {
            if (payload.data[i].boat in sessionsForBoats) {
              sessionsForBoats[payload.data[i].boat].push(payload.data[i]);
            } else {
              sessionsForBoats[payload.data[i].boat] = [ payload.data[i] ];
            }
            curves[payload.data[i]._id] = payload.data[i];
          }
          
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
    };
  });
