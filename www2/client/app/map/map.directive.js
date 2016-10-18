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

          setTimeout(function() { scope.$apply(); }, 10);
        });
      }
    };
  })
  .directive('heightCheck', function ($timeout) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        var adjustHeight = function(el) {
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
        }
        element.on('click', function() {
          adjustHeight(angular.element(this));
        });
        angular.element(window).on('resize', function() {
          adjustHeight(angular.element('label.height-check'));
        });
      }
    };
  })
  .directive('widthCheck', function ($window) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        var win = angular.element($window);
        var changeWidth = function(element) {
          angular.element(element).each(function(i, e) {
            var el = angular.element(e);
            var target = angular.element(el.attr('target'));

            el.outerWidth(target.outerWidth());
          });
        }

        setTimeout(function() { changeWidth(element); }, 100 );
        
        win.on('resize', function() {
          changeWidth(element);
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
  })
  .directive('btnToggle', function () {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function() {
          var el = angular.element(this);
          var target = angular.element('.res-container.'+el.attr('target'));

          angular.element('.responsiveNavButtons .res-container').not(target).hide();
          target.toggle();

          if(target.hasClass('res-photos')) {
            var tabs = target.find('#tabs');
            target.find('hr').outerWidth(tabs.outerWidth());
          }

          setTimeout(function() { scope.$apply(); }, 10);
        });
      }
    };
  })
  .directive('movingElement', function ($window) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        var win = angular.element($window);
        var moveElements = function(element) {
          angular.element(element).each(function(i, e) {
            var el = angular.element(e);
            var parent = $window.innerWidth <= 799 ? el.attr('mobile-parent') : el.attr('desktop-parent');
            parent = angular.element(parent);
            
            el.appendTo(parent);

            setTimeout(function() { scope.$apply(); }, 10);
          });
        }

        setTimeout(function() { moveElements(element); }, 100 );
        
        win.on('resize', function() {
          moveElements(element);
        });
      }
    };
  });
