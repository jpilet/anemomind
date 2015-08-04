'use strict';

var app = angular.module('www2App')
  .directive('boatSummary', function (boatList) {
    return {
      templateUrl: 'app/boatSummary/boatSummary.html',
      restrict: 'EA',
      scope: {
        boats: "@",
        sessions: "@"
      },
      link: function (scope, element, attrs) {
        scope.boats = boatList.boats();
        scope.sessions = boatList.sessions();
        scope.$on('boatList:updated', function(event, boats) {
          scope.boats = boats;
        });
        scope.$on('boatList:sessionsUpdated', function(event, data) {
          scope.sessions = data;
        });
        scope.boat = boatList.boat;
        scope.currentPage = 0;
        scope.pageSize = 10;
        scope.numberOfPages = function() {
          return Math.ceil(scope.data.length/scope.pageSize);                
        }
      }
    };
  });

app.filter('startFrom', function() {
    return function(input, start) {
        start = +start; //parse to int
        if (input) return input.slice(start);
    }
});
