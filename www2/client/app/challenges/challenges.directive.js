'use strict';

angular.module('www2App')
  .directive('withPopover', function () {
    return {
      restrict: 'C',
      link: function (scope, element, attrs) {
        angular.element(element).on('click', function (e) {
            angular.element('.with-popover').not(this).each(function(i, e) {
                angular.element(e).parent().find('div').addClass('hidden');
            });
            angular.element(e).parent().find('div').removeClass('hidden');
        });
      }
    };
  })
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
  })
  .directive('challengeContainer', function ($window, $timeout) {
    return {
      restrict: 'C',
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
              responsive: [
                {
                  breakpoint: 768,
                  settings: {
                    slidesToShow: 3
                  }
                },
                {
                  breakpoint: 480,
                  settings: {
                    slidesToShow: 2
                  }
                },
                {
                  breakpoint: 380,
                  settings: {
                    slidesToShow: 1
                  }
                }
              ]
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

  
