'use strict';

angular.module('www2App')
.directive('gaugeIndicator', function () {
    return {
      templateUrl: 'app/gaugeIndicator/gaugeIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
        description: "=",
        min: "=",
        max: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new Panel(element);

        function update(){
          var rotation=($scope.value-$scope.min)*180/($scope.max-$scope.min);
          panel.updatePanelGraphs(rotation);
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});