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
        }, true);
        $scope.$watch('currentTime', function(newValue, oldValue) {
          graph.setTimeMarks([newValue]);
        }, true);
        $scope.$watch('plotField', function(newValue, oldValue) {
          graph.setData(newValue, $scope.plotData);
        });
      }
    };
  });
