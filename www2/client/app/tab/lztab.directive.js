'use strict';

angular.module('www2App')
.directive('lztab', function () {
  return {
    restrict: 'E',
    replace: true,
    require: '^lztabset',
    scope: {
      title: '@',
      templateUrl: '@'
    },
    link: function(scope, element, attrs, tabsetController) {
      tabsetController.addTab(scope);

      scope.select = function () {
        tabsetController.selectTab(scope);
      }
      //allows lazy loading
      scope.$watch('selected', function () {
        if (scope.selected) {
          tabsetController.setTabTemplate(scope.templateUrl);
        }
      });
    },
    template:
      '<li ng-class="{active: selected}">' +
        '<a href="" ng-click="select()">{{ title }}</a>' +
      '</li>'
  };
});