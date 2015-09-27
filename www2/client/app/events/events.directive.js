'use strict';

angular.module('www2App')
  .directive('events', function ($http, Auth) {
    return {
      templateUrl: 'app/events/events.html',
      restrict: 'E',
      scope: {
        boat: '=',
        before: '=',
        after: '=',
        currentTime: '='
      },
      link: function (scope, element, attrs) {
        scope.events = [];
        $http.get('/api/events', { params: {
            b: scope.boat,
            A: scope.after.toISOString(),
            B: scope.before.toISOString()
          }})
          .success(function(data, status, headers, config) {
            if (status == 200) {
              // Parse date
              for (var i in data) {
                var event = data[i];
                event.when = new Date(event.when);
              }
              scope.events = data;
            }
          });
        scope.photoUrl = function(event, size) {
          return '/api/events/photo/' + event.boat + '/' + event.photo
            + '?' + (size? 's=' + size + '&' : '') + 'access_token=' + Auth.getToken() ;
        };
        scope.thumbnail = function (event) {
          return scope.photoUrl(event, '120x120');
        };
        scope.onTimeSelect = function(when) {
          scope.currentTime = when;
        };
      }
    };
  });
