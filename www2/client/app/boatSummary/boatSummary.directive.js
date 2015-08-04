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
      }
    };
  });

app.filter('startFrom', function() {
    return function(input, start) {
        start = +start; //parse to int
        if (input) return input.slice(start);
    }
});
