'use strict';

angular.module('www2App')
  .directive('expandable', function () {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function() {
          var el = angular.element(this);
          var icon = el.find('i');
          var toggleClasses = el.attr('toggle-classes');
          var colState = icon.attr('col-state') === 'true';
          var newClass = colState ? icon.attr('uncol-class') : icon.attr('col-class');
          var parent = el.parents(icon.attr('col-parent'));
          var target = parent.find(icon.attr('col-target'));
          
          icon.removeClass(icon.attr('col-class')+' '+icon.attr('uncol-class')).addClass(newClass);
          icon.attr('col-state', String(!colState));
          target.toggleClass(toggleClasses);

          if(el.attr('mode') == 'hide-parent') {
            parent.toggleClass(parent.attr('toggle-classes'));
          }
        });
      }
    };
  });
