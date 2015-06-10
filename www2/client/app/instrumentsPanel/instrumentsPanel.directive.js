'use strict';

angular.module('www2App')
.directive('instrumentsPanel', function () {
    return {
      templateUrl: 'app/instrumentsPanel/instrumentsPanel.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new Panel();

        $scope.$watch('value', function(newValue, oldValue) {
          panel.updatePanelGraphs($scope.value);
      }, true);
    }
};
});