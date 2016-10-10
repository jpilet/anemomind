'use strict';

angular.module('www2App')
  .directive('withPopover', function ($http, Auth, userDB, $httpParamSerializer) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function (e) {
            angular.element('.with-popover').not(this).each(function(i, e) {
                angular.element(e).parent().find('div').hide();
            })
        });
      }
    };
  });
