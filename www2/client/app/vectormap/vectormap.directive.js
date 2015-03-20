'use strict';

angular.module('www2App')
  .directive('vectormap', function ($timeout, $window) {
    return {
      template: '<canvas style="width:100%;height:100%"></canvas>',
      restrict: 'E',
      link: function (scope, element, attrs) {
        var canvas = new CanvasTilesRenderer({
          canvas: element.children()[0],
          url: function(scale, x, y) { 
            return "http://b.tile.opencyclemap.org/cycle/" + scale + "/" + x + "/" + y + ".png";
          },
          maxNumCachedTiles: 256,
          onLocationChange: function(canvasTilesRenderer) {
            $timeout(function() {
              scope.mapLocation = canvasTilesRenderer.getLocation();
            });
          }
        });
        scope.pathLayer = canvas.layers[1];
        scope.selectedCurve = undefined;
        scope.plotData = [];
        scope.plotField = 'gpsSpeed';
        scope.currentTime = undefined;

        scope.pathLayer.onSelect = function(curveId) {
          $timeout(function() {
            scope.selectedCurve = curveId;
            scope.plotData = scope.pathLayer.getPointsForCurve(curveId);
          });
        };

        scope.pathLayer.onDataArrived = function() {
          $timeout(function() {
            if (scope.selectedCurve) {
              scope.plotData = scope.pathLayer.getPointsForCurve(scope.selectedCurve);
            }
          });
        };

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

        angular.element($window).bind('resize', function () {
          scope.$apply();
        });

        // A clic on the map selects a curve and sets current time.
        canvas.pinchZoom.onClic = function(pos) {
          var point = scope.pathLayer.findPointAt(
            pos.startWorldPos.x, pos.startWorldPos.y);
          if (point) {
            scope.selectedCurve = point.curveId;
            scope.currentTime = point.point.time;
          } else {
            scope.selectedCurve = undefined;
            scope.currentTime = undefined;
          }
          scope.$apply();
        };

        scope.$watch('selectedCurve', function(newValue, oldValue) {
          if (newValue != oldValue) {
            scope.pathLayer.selectCurve(newValue);
            scope.plotData = scope.pathLayer.getPointsForCurve(newValue);
          }
        });

        scope.$watch('currentTime', function(newValue, oldValue) {
          if (newValue != oldValue) {
            scope.pathLayer.setCurrentTime(newValue);
          }
        });
      }
    };
  });
