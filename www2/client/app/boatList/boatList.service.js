'use strict';

angular.module('www2App')
  .service('boatList', function (Auth, $http, $q,socket, $rootScope) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};

    var deferred = $q.defer();
    //
    // return promise on this.boats();
    var promise;


    //
    // return the default boat to display at home
    function getDefaultBoat() {
      if(!boats.length){
        return {};
      }

      // default super implementation
      return boats[boats.length-1];
    }

    function update() {
        promise = deferred.promise;
        // Specifically ask for public boat, too.
        // We could have a user option to hide public boats.
        $http.get('/api/boats?public=1')
          .success(function(data, status, headers, config) {
             boats = data;
             socket.syncUpdates('boat', boats);

             for (var i in data) {
               var boat = data[i];
               boatDict[boat._id] = boat;
             }
             //
             // resolve
             deferred.resolve(boats);
             $rootScope.$broadcast('boatList:updated', boats);
          });
        $http.get('/api/session')
          .success(function(data, status, headers, config) {
            sessionsForBoats = [];
            for (var i in data) {
              if (data[i].boat in sessionsForBoats) {
                sessionsForBoats[data[i].boat].push(data[i]);
              } else {
                sessionsForBoats[data[i].boat] = [ data[i] ];
              }
              curves[data[i]._id] = data[i];
            }
            $rootScope.$broadcast('boatList:sessionsUpdated', sessionsForBoats);
          });
    }

    update();
    //
    // TODO you can replace this watch by a broadcast/listen event 
    $rootScope.$watch(Auth.isLoggedIn, function(newVal, oldVal) {
      if (newVal && newVal != oldVal) {
        update();
      }
    });

    function locationForCurve(curveId) {
      if (!(curveId in curves)) {
        return undefined;
      }
      var c = curves[curveId];
      return c.location;
    }

    return {
      boat: function(id) { return boatDict[id]; },
      boats: function() { return promise; },
      sessions: function() { return $.extend({}, sessionsForBoats); },
      sessionsForBoat: function(boatId) { return sessionsForBoats[boatId]; },
      getCurveData: function(curveId) { return curves[curveId]; },
      getDefaultBoat: getDefaultBoat,
      locationForCurve: locationForCurve,
      update: update,
    };
  });
