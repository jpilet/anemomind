'use strict';

angular.module('www2App')
  .directive('vectormap', function ($timeout) {
    return {
      template: '<canvas style="width:100%;height:100%"></canvas>',
      restrict: 'E',
      link: function (scope, element, attrs) {
        var canvas = new CanvasTilesRenderer({
          canvas: element.children()[0],
          url: function(scale, x, y) { 
            return "http://b.tiles.mapbox.com/v3/openplans.map-g4j0dszr/" + scale + "/" + x + "/" + y + ".png";
            // return "http://b.tile.opencyclemap.org/cycle/" + scale + "/" + x + "/" + y + ".png";
          },
          maxNumCachedTiles: 256,
          onLocationChange: function(canvasTilesRenderer) {
            $timeout(function() {
              scope.mapLocation = canvasTilesRenderer.getLocation();
            });
          }
        });

        scope.$watch('mapLocation', function(newValue, oldValue) {
          function near(x,y) {
            if (!x || !y) return false;
            return Math.abs(y-x) < 1e-10;
          }
          if (!newValue) {
            return;
          }
          if (!oldValue
              || !near(newValue.x, oldValue.x)
              || !near(newValue.y, oldValue.y)
              || !near(newValue.scale, oldValue.scale)) {
            canvas.setLocation(newValue);
          }
        }, true);


        scope.$watch('boat._id', function(newValue, oldValue) {
          if (newValue && newValue != oldValue) {
            canvas.layers[1].setUrl(function (scale, x, y) {
              return "/api/tiles/raw/" + scale + '/' + x + '/' + y + '/' + newValue + '/';
            });
          }
        });

        // Watch for resize
        scope.$watch(
          function () {
            return {
              w: element.width(),
              h: element.height()
            };
          },
          function (newValue, oldValue) {
            if (newValue.w != oldValue.w || newValue.h != oldValue.h) {
              canvas.resizeCanvas();
            }
          },
          true // deep object compare
          );
      }
    };
  });
