'use strict';

angular.module('www2App')
  .directive('events', function ($http, Auth, userDB, Lightbox, $httpParamSerializer) {
    return {
      templateUrl: 'app/events/events.html',
      restrict: 'E',
      scope: {
        boat: '=',
        before: '=',
        after: '=',
        currentTime: '=',
        events: '='
      },
      link: function (scope, element, attrs) {
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
        
        scope.openLightboxModal = function(index) {
          var images = [];
          angular.forEach(scope.events, function(value, key) {
            if(typeof value.photo !== 'undefined' && value.photo && value.photo != null) {
              var image = {
                'url': scope.photoUrl(value, ''),
                'caption': value.comment
              };
              images.push(image);
            }
          });

          Lightbox.openModal(images, index);
        }
      }
    };
  });
