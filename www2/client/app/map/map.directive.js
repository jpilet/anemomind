'use strict';
angular.module('www2App')
  .directive('autoSelect', ['$window', function ($window) {
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
  }]);
