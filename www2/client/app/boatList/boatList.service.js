'use strict';

angular.module('www2App')
  .service('boatList', function (Auth, $http, socket, $rootScope) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};

    function update() {
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
      boats: function() { return boats.slice(0); },
      sessions: function() { return $.extend({}, sessionsForBoats); },
      sessionsForBoat: function(boatId) { return sessionsForBoats[boatId]; },
      getCurveData: function(curveId) { return curves[curveId]; },
      locationForCurve: locationForCurve,
      update: update,
    };
  });
