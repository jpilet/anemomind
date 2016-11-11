'use strict';

angular.module('www2App')
  .directive('inputSelectText', ['$window', function ($window) {
      return {
          restrict: 'A',
          link: function (scope, element, attrs) {
          		element.keypress(function( event ) {
					     event.preventDefault();
					   	});
              element.on('click', function () {
                  if (!$window.getSelection().toString()) {
                      // Required for mobile Safari
                      this.setSelectionRange(0, this.value.length)
                  }
              });
          }
      };
  }])
  .directive('checkHeight', function ($window, Lightbox) {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        var win = angular.element($window);
        var el = angular.element(element);
        var icon = el.find('label i');
        var offset = 35; // We should also make the arrow button visible

        // Checks if the container is inside the map
        var isInsideTheMap = function(elem) {
          var view = angular.element('.vectorMapContainer');
          var docViewTop = view.scrollTop();
          var docViewBottom = docViewTop + view.height();

          var elemTop = $(elem).offset().top;
          var elemBottom = elemTop + $(elem).outerHeight(true);

          return ((elemBottom <= docViewBottom) && (elemTop >= docViewTop));
        };

        var adjustHeight = function() {
          var infoHeight = el.outerHeight(true);
          var windowHeight = angular.element('.vectorMapContainer').outerHeight();
          var bottomPos = el.offset().top + infoHeight;

          el.css('height','auto').removeClass('scrollable');

          if(!isInsideTheMap(el.parents('.mainControl'))) {
            el.outerHeight((infoHeight - (bottomPos - windowHeight)) - offset).addClass('scrollable');
          }
        };

        // I used specific triggers because $watch
        // has some delays when capturing the values of an element
        icon.on('click', function() {
          adjustHeight();          
        });
        angular.element('.graph .extension').on('click', function() {
          adjustHeight();
        });
        win.on('resize', function() {
          adjustHeight();
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

          Lightbox.templateUrl = 'app/map/map.social.html';
          Lightbox.openModal(images,0);

          Lightbox.templateUrl = oldTemplate;
        });
      }
    };
  })
  .directive('movingElement', function ($window, $timeout) {
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

            $timeout(function() { scope.$apply(); }, 10);
          });
        }

        // As observed, SVG's with a definite size does render properly if the containers are too small
        // We have to let the CSS styles kick in first.
        $timeout(function() {
          moveElements(element);
        }, 100);
        
        win.on('resize', function() {
          $timeout(function() {
            moveElements(element);
          }, 100);
        });
      }
    };
  });
