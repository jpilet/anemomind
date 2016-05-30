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
            A: (scope.after ? scope.after.toISOString() : undefined),
            B: (scope.before ? scope.before.toISOString() : undefined)
          }})
          .success(function(data, status, headers, config) {
            scope.events = [];
            if (status == 200) {
              var times= {};
              for (var i in data) {
                var event = data[i];
                // Parse date
                event.when = new Date(event.when);

                // Fetch user details
                userDB.resolveUser(event.author, function(user) {
                  scope.users[user._id] = user;
                });

                // Remove duplicates
                var key = "" + event.when.getTime();
                if (!(key in times)) {
                  times[key] = 1;
                  scope.events.push(event);
                }
              }
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
