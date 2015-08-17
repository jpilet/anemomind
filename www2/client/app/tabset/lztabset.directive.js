'use strict';

angular.module('www2App')
  .directive('lztabset', function () {
  return {
    restrict: 'E',
    replace: true,
    transclude: true,
    controller: function($scope) {
      $scope.templateUrl = '';
      var tabs = $scope.tabs = [];
      var controller = this;

      this.selectTab = function (tab) {
        angular.forEach(tabs, function (tab) {
          tab.selected = false;
        });
        tab.selected = true;
      };

      this.setTabTemplate = function (templateUrl) {
        $scope.templateUrl = templateUrl;
      }

      this.addTab = function (tab) {
        if (tabs.length == 0) {
          controller.selectTab(tab);
        }
        tabs.push(tab);
      };
    },
    template:
      '<div class="row-fluid">' +
        '<div>' +
          '<div class="nav nav-tabs" ng-transclude></div>' +
        '</div>' +
        '<div>' +
          '<ng-include src="templateUrl">' +
        '</ng-include></div>' +
      '</div>'
  };
});


