angular.module('www2App')
.directive('arrowIndicator', function () {
    return {
      templateUrl: 'app/arrowIndicator/arrowIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
        description: "=",
        to: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new ArrowPanel(element);

        function update(){
          if($scope.to){
            rotation= $scope.value-360;
            if($scope.value < 180){
              rotation = rotation + 180;
            }
          }
          else{
            rotation=$scope.value;
          }
          panel.updatePanelGraphs(rotation);
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});