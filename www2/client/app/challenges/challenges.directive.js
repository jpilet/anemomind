'use strict';

angular.module('www2App')
  .directive('heading', function () {
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

  
