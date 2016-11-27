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
  })
  .directive('heading', function ($http, Auth, userDB, $httpParamSerializer) {
    return {
      restrict: 'C',
      scope: {
        reverse: '=',
      },
      link: function (scope, element, attrs) {
        var sort = '';
        var el = angular.element(element);
        
        el.on('click', function (e) {
            var _this = angular.element(this);

            sort = scope.reverse ? 'up' : 'down';
            el.siblings().removeClass('up down');
            _this.removeClass('up down').addClass(sort);
        });
      }
    };
  });

  
