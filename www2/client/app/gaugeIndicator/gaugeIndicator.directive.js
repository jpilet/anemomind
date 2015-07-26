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
        var panel=new GaugePanel(element);

        function update(){
          var diff = $scope.max-$scope.min;
          if(diff != 0){
            var rotation=($scope.value-$scope.min)*180/($scope.max-$scope.min);
            panel.updatePanelGraphs(rotation);
          }
          else{
            panel.updatePanelGraphs($scope.value);
          }
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});