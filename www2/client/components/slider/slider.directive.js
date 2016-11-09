'use strict';

angular.module('www2App')
  .directive('isSlickSlider', function ($window, $timeout) {
    return {
      restrict: 'C',
      scope: {
        breakpoints: '='
      },
      link: function (scope, element, attrs) {
        var sort = '';
        var el = angular.element(element);
        var initSlick = function(el) {
          if($window.outerWidth <= 768) {
            el.slick({
              centerMode: true,
              arrows: true,
              centerPadding: '40px',
              slidesToShow: 3,
              prevArrow: '<button type="button" class="slick-arrow slick-prev">&nbsp;</button>',
              nextArrow: '<button type="button" class="slick-arrow slick-next">&nbsp;</button>',
              responsive: scope.breakpoints
            });
          }
          else {
            if(el.hasClass('slick-slider'))
              el.slick('unslick');
          }
        }
        
        // same as document.ready
        $timeout(function() { initSlick(el); }, 100);
        angular.element($window).on('resize', function() {
          initSlick(el);
        });
      }
    };
  });

  
