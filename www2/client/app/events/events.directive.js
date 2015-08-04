'use strict';

angular.module('www2App')
  .directive('events', function ($http) {
    return {
      templateUrl: 'app/events/events.html',
      restrict: 'E',
      scope: {
        boat: '='
      },
      link: function (scope, element, attrs) {
        scope.events = [];
        $http.get('/api/events?b=' + scope.boat)
          .success(function(data, status, headers, config) {
            if (status == 200) {
              scope.events = data;
            }
          });
      }
    };
  });
