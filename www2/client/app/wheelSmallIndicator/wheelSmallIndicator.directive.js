'use strict';

angular.module('www2App')
.directive('wheelSmallIndicator', function () {
    return {
      templateUrl: 'app/wheelSmallIndicator/wheelSmallIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
        description: "=",
        min: "=",
        max: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new WheelSmallPanel(element);

        function update(){
          var rotation=($scope.value-$scope.min)*360/($scope.max-$scope.min);
          panel.updatePanelGraphs(rotation);
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});