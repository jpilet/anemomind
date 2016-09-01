'use strict';

angular.module('www2App')
  .directive('events', function ($http, Auth, userDB, $httpParamSerializer) {
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
          var url = [
            '/api/events/photo/' + event.boat + '/' + event.photo,
            $httpParamSerializer({s : size, access_token: Auth.getToken()})
          ];
          return url.join('?');
        };
        scope.thumbnail = function (event) {
          return scope.photoUrl(event,
             // Load more pixels if the screen can handle it.
             (window.devicePixelRatio && window.devicePixelRatio >= 2 ?
              '700' : '400')
             + 'x_');
        };
        scope.onTimeSelect = function(when) {
          scope.currentTime = when;
        };
      }
    };
  });
