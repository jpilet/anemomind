'use strict';

angular.module('www2App')
.directive('wheelSmallIndicator', function () {
    return {
      templateUrl: 'app/wheelSmallIndicator/wheelSmallIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        arrow: "=",
        boat: "=",
        north: "=",
        description: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new WheelSmallPanel(element);

        function update(){

          panel.updatePanelGraphs($scope.arrow, $scope.boat,$scope.north);
        }
        $scope.$watch('arrow', update);
        $scope.$watch('boat', update);
        $scope.$watch('north', update);

    }
};
});