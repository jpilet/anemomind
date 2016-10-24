'use strict';

angular.module('www2App')
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
