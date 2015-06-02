'use strict';

angular.module('www2App')
.directive('instrumentsPanel', function () {
    return {
      templateUrl: 'app/instrumentsPanel/instrumentsPanel.html',
      restrict: 'EA',
      link: function ($scope, element, attrs) {
        createPanel();

        $scope.$watch('currentTime', function(newValue, oldValue) {
          updateGraphs();
      }, true);
    }
};
});