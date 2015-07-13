angular.module('www2App')
.directive('arrowIndicator', function () {
    return {
      templateUrl: 'app/arrowIndicator/arrowIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
        description: "=",
        min: "=",
        max: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new ArrowPanel(element);

        function update(){
          var rotation = ($scope.value-$scope.min)*360/($scope.max-$scope.min);

          //Not so sure about how to match the TWA on map
          rotation= rotation-360;
          if($scope.value < 180){
            rotation = rotation + 180;
          }
          panel.updatePanelGraphs(rotation);
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});