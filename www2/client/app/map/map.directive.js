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
  })
  .directive('heightCheck', function ($timeout) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        element.on('click', function() {
          var el = angular.element(this);
          var parent = el.parent();
          var container = parent.find('.collapsible');
          $timeout(function() {
            if(container.hasClass('uncollapse-area')) {
              var parentPos = parent.offset();
              var resBtnsPos = angular.element('.responsiveNavButtons').offset();
              var heightTotal = parent.height() + parentPos.top;

              // checks if the container overlaps the footer
              if(heightTotal > resBtnsPos.top) {
                parent.outerHeight(resBtnsPos.top - parentPos.top);
                
                container.outerHeight(parent.outerHeight() - el.outerHeight());
                container.css('overflow-y','scroll');
              }
            }
            else {
              parent.attr('style','');
              container.attr('style','');
            }
          }, 100);
        });
      }
    };
  })
  .directive('shareLightbox', function (Lightbox) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function() {
          // To fool the Lightbox that there is an image. 
          // Sadly, this Lightbox plugin doesn't play friendly if there is no image passed.
          var images = [{'url': '',}]; 

          var oldTemplate = Lightbox.templateUrl;
          
          Lightbox.templateUrl = 'app/map/sharePopup.html';
          Lightbox.openModal(images,0);

          Lightbox.templateUrl = oldTemplate;
        });
      }
    };
  });
