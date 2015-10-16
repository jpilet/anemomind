'use strict';

angular.module('www2App')
  .directive('perfplot', function ($timeout) {
    return {
      template: '<div></div>',
      restrict: 'E',
      scope: {
        plotData: '=',
        currentTime: '=',
        plotField: '=field'
      },
      link: function ($scope, element, attrs) {
        var graph = new Graph(element[0]);
        graph.setData($scope.plotField, $scope.plotData);

        graph.onTimeClick = function(time) {
          $timeout(function() {
            $scope.currentTime = time;
          });
        };

        $scope.$watch('plotData', function(newData, oldData) {
          graph.setData($scope.plotField, newData);
        }, false);
        $scope.$watch('currentTime', function(newValue, oldValue) {
          if (newValue != undefined) {
            graph.setTimeMarks([newValue]);
          } else {
            graph.setTimeMarks([]);
          }
        }, true);
        $scope.$watch('plotField', function(newValue, oldValue) {
          graph.setData(newValue, $scope.plotData);
        });

        // Watch for resize
        $scope.$watch(
          function () {
            return {
              w: element.width(),
              h: element.height()
            };
          },
          function (newValue, oldValue) {
            if (newValue.w != oldValue.w || newValue.h != oldValue.h) {
              graph.draw();
            }
          },
          true // deep object compare
        );
      }
    };
  });
