'use strict';

angular.module('www2App')
.directive('heelingIndicator', function () {
    return {
      templateUrl: 'app/heelingIndicator/heelingIndicator.html',
      restrict: 'E',
      scope: {
        label: "=",
        value: "=",
        description: "=",
      },
      link: function ($scope, element, attrs) {
        var panel=new HeelingPanel(element);

        function update(){
          var rotation=$scope.value;
          panel.updatePanelGraphs(rotation);
        }
        $scope.$watch('value', update);
        $scope.$watch('min', update);
        $scope.$watch('max', update);

    }
};
});