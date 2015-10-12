'use strict';

angular.module('www2App')
  .directive('events', function ($http, Auth, userDB) {
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
        scope.users = {};
        $http.get('/api/events', { params: {
            b: scope.boat,
            A: scope.after.toISOString(),
            B: scope.before.toISOString()
          }})
          .success(function(data, status, headers, config) {
            if (status == 200) {
              for (var i in data) {
                var event = data[i];
                // Parse date
                event.when = new Date(event.when);

                // Fetch user details
                userDB.resolveUser(event.author, function(user) {
                  scope.users[user._id] = user;
                });
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
