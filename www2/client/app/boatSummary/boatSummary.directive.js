'use strict';

var app = angular.module('www2App')
  .directive('boatSummary', function (boatList) {
    return {
      templateUrl: 'app/boatSummary/boatSummary.html',
      restrict: 'E',
      scope: {
        boatId: "=",
        pageSize: "=?"
      },
      link: function (scope, element, attrs) {
        scope.currentPage = 1;
        scope.sessions = [];
        if (scope.pageSize == undefined) {
          scope.pageSize = 10;
        }
        function updateSessions() {
          if (!scope.boatId) {
            return;
          }
          scope.boat = boatList.boat(scope.boatId);
          scope.sessions = boatList.sessionsForBoat(scope.boatId);
          if (scope.sessions == undefined) {
            scope.sessions = [];
          }
        }
        scope.$on('boatList:updated', updateSessions);
        scope.$on('boatList:sessionsUpdated', updateSessions);
        scope.$watch('boatId', updateSessions);

        updateSessions();

        scope.twdirToCardinal = function(twdir) {
          var index = Math.round(360 + twdir * 8 / 360) % 8;
          var windrose = [
            "N", "NE", "E", "SE", "S", "SW", "W", "NW" ];
          return windrose[index];
        };
        scope.formatSpan = function(fromDateS, toDateS) {
          var fromDate = new Date(fromDateS);
          var toDate = new Date(toDateS);
          function makeDurString(durSeconds) {
            var dur = durSeconds;
            var multipliers = [60, 60, 24, 7, 1000000000];
            var labels = ['second', 'minute', 'hour', 'day', 'week'];
            var formatDur = function(x, s) {
              return (x == 0? null : x + ' ' + s + (x == 1? "" : "s"));
            }
            var dst = [];
            for (var i = 0; i < multipliers.length; i++) {
              var f = multipliers[i];
              var a = Math.floor(dur/f);
              var b = Math.floor(dur - f*a);
              var s = formatDur(b, labels[i]);
              if (s) {
	              dst.push(s);
              }
              dur = a;
            }
            var finalParts = dst.reverse();
            if (finalParts.length == 0) {
              return "0 seconds";
            } else if (finalParts.length == 1) {
	            return finalParts[0];
            } else {
              return finalParts[0] + ' and ' + finalParts[1];
            }
          }

          var offsetIso = fromDate.toISOString();

          // What is both an easily readable and 
          // somewhat standard format for dates? 
          // Raw ISO dates are standard,
          // but look a bit technical in this context, 
          // so let's declutter it a bit...
          var offsetString = offsetIso.substring(0, 10) 
              + " " + offsetIso.substring(11, 16);
          var durSeconds = 0.001*(toDate.getTime() - fromDate.getTime());
          return offsetString + " (" + makeDurString(durSeconds) + ")";
        }

        scope.knotsToBeaufort = function(knots) {
          if (knots < 1) { return 0; }
          if (knots < 3) { return 1; }
          if (knots < 6) { return 2; }
          if (knots < 10) { return 3; }
          if (knots < 16) { return 4; }
          if (knots < 21) { return 5; }
          if (knots < 27) { return 6; }
          if (knots < 33) { return 7; }
          if (knots < 40) { return 8; }
          if (knots < 47) { return 9; }
          if (knots < 55) { return 10; }
          if (knots < 63) { return 11; }
          return 12;
        };
      }
    };
  });

app.filter('startFrom', function() {
    return function(input, start) {
        start = +start; //parse to int
        if (input) return input.slice(start);
    }
});
