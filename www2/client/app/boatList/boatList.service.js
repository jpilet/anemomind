'use strict';

angular.module('www2App')
  .service('boatList', function (Auth, $http, socket, $rootScope) {
    var boats = [ ];
    var boatDict = { };
    var curves = { };
    var sessionsForBoats = {};

    function update() {
        $http.get('/api/boats')
          .success(function(data, status, headers, config) {
             boats = data;
             socket.syncUpdates('boat', boats);

             // Load sessions 
             for (var i in data) {
               var boat = data[i];
               boatDict[boat._id] = boat;
               loadSessionsForBoat(boats[i]);
             }

             $rootScope.$broadcast('boatList:updated', boats);
          });
    }

    function loadSessionsForBoat(boat) {
      $http.get('/api/tiles/raw/0/0/0/' + boat._id)
      .success(function(data, status, headers, config) {
        for (var i in data) {
          var element = data[i];
          if (!(element.boat in sessionsForBoats)) sessionsForBoats[element.boat] = [];
          sessionsForBoats[element.boat].push(data[i]);
          for (var c in element.curves) {
            var curve = element.curves[c];
            if (curve.curveId in curves) {
              curves[curve.curveId].push(curve);
            } else {
              curves[curve.curveId] = [curve];
            }
          }
        }
        sessionsForBoats[element.boat].reverse();
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

      var curveElements = curves[curveId];

      var minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;
      for (var e in curveElements) {
        var element = curveElements[e];
        for (var i in element.points) {
          var p = element.points[i].pos;
          minX = Math.min(p[0], minX);
          minY = Math.min(p[1], minY);
          maxX = Math.max(p[0], maxX);
          maxY = Math.max(p[1], maxY);
        }
      }
      return {
        x: (minX + maxX) / 2,
        y: (minY + maxY) / 2,
        scale: 2*Math.max(maxX - minX, maxY - minY)
      };
    }

    return {
      boat: function(id) { return boatDict[id]; },
      boats: function() { return boats.slice(0); },
      sessions: function() { return $.extend({}, sessionsForBoats); },
      getCurveData: function(curveId) { return curves[curveId]; },
      locationForCurve: locationForCurve,
      update: update,
    };
  });
