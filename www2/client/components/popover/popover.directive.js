'use strict';

angular.module('www2App')
  // To hide other pop-ups when
  // a current popover is clicked
  .directive('withPopover', function () {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function (e) {
            angular.element('.with-popover').not(this).each(function(i, e) {
                angular.element(e).parent().find('div.popover').addClass('hidden');
            });
            angular.element(e).parent().find('div.popover').removeClass('hidden');
        });
      }
    };
  });
