
angular.module('www2App')
.directive('wheelIndicator', function () {
    return {
      templateUrl: 'app/wheelIndicator/wheelIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        arrow: "=",
        north: "=",
        description: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new WheelPanel(element);

        function update(){

          panel.updatePanelGraphs($scope.arrow, $scope.north);
          panel.updatePanelText($scope.arrow);
        }
        $scope.$watch('arrow', update);
        $scope.$watch('north', update);

    }
};
});