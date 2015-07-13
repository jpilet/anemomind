'use strict';

angular.module('www2App')
  .directive('wheelSmallIndicator', function () {
    return {
      templateUrl: 'app/wheelSmallIndicator/wheelSmallIndicator.html',
      restrict: 'EA',
      link: function (scope, element, attrs) {
      }
    };
  });