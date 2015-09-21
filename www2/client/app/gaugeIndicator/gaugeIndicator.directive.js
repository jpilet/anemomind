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
          if($scope.value != undefined && !isNaN($scope.value)){
            var min = $scope.min || 0;
            var max = $scope.max || 100;
            var diff = max - min;
            var val = Math.min(max, Math.max(min, $scope.value));
            var rotation=(val-min)*180/(max-min);
            panel.updatePanelGraphs(rotation);
          } else {
            panel.updatePanelGraphs(0);
          }
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});
